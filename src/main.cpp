// =============================================================================
// visionCore - точка входа CLI (Фаза 3).
//
// Запускает полный pipeline: шум -> детекция -> признаки -> форма -> разметка.
// =============================================================================

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "visionCore/core/Image.hpp"
#include "visionCore/domain/ShapeKind.hpp"
#include "visionCore/infra/AppConfig.hpp"
#include "visionCore/infra/Logger.hpp"
#include "visionCore/io/FileWalker.hpp"
#include "visionCore/io/ImageIO.hpp"
#include "visionCore/io/PathUtils.hpp"
#include "visionCore/pipeline/PipelineFactory.hpp"
#include "visionCore/pipeline/PipelineResult.hpp"

namespace {

using namespace visionCore;

/**
 * @brief Параметры командной строки.
 */
struct CliOptions {
    std::filesystem::path inputPath;
    bool saveNoiseFree = false;
    bool saveOutline = true;
    bool showHelp = false;
};

void printUsage(const char* programName) {
    std::cout
        << "Использование:\n"
        << "  " << programName << " <путь к файлу или папке> [опции]\n"
        << "\n"
        << "Опции:\n"
        << "  --save-noise-free    сохранить очищенное изображение (*-noiseFree)\n"
        << "  --save-outline       сохранить разметку (*-shapeOutline), по умолчанию вкл.\n"
        << "  --no-outline         не сохранять разметку\n"
        << "  -h, --help           справка\n"
        << "\n"
        << "Поддерживаемые форматы: .png .jpg .jpeg .bmp .tif .tiff\n";
}

CliOptions parseArgs(int argc, char* argv[]) {
    CliOptions opts;
    if (argc < 2) {
        opts.showHelp = true;
        return opts;
    }

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            opts.showHelp = true;
        } else if (arg == "--save-noise-free") {
            opts.saveNoiseFree = true;
        } else if (arg == "--save-outline") {
            opts.saveOutline = true;
        } else if (arg == "--no-outline") {
            opts.saveOutline = false;
        } else if (arg.front() == '-') {
            throw std::runtime_error("Неизвестный аргумент: " + arg);
        } else if (opts.inputPath.empty()) {
            opts.inputPath = arg;
        } else {
            throw std::runtime_error("Указано несколько путей: " + arg);
        }
    }

    if (!opts.showHelp && opts.inputPath.empty()) {
        opts.showHelp = true;
    }

    return opts;
}

void logObject(const pipeline::ProcessedObject& obj) {
    const auto& d = obj.detection;
    const auto kind = obj.classification.topShape();
    const float conf = obj.classification.topConfidence();

    std::ostringstream oss;
    oss << "  Объект #" << d.id << ": "
        << domain::toString(kind) << " (" << std::fixed
        << std::setprecision(0) << (conf * 100.0f) << "%)\n"
        << "    bbox: (x=" << d.bbox.x << ", y=" << d.bbox.y
        << ", w=" << d.bbox.width << ", h=" << d.bbox.height << ")\n"
        << "    центроид: (" << d.centroid.x << ", " << d.centroid.y << ")\n"
        << "    площадь: " << static_cast<int>(d.area) << " px^2";

    infra::Logger::info(oss.str());

    if (obj.classification.scores.size() > 1) {
        std::ostringstream dist;
        dist << "    распределение:";
        for (const auto& [k, v] : obj.classification.scores) {
            dist << " " << domain::toString(k) << "="
                 << std::fixed << std::setprecision(0) << (v * 100.0f) << "%";
        }
        infra::Logger::info(dist.str());
    }
}

void processFile(const std::filesystem::path& file,
                 const CliOptions& opts,
                 const infra::AppConfig& config,
                 pipeline::Pipeline& pipeline) {
    const core::Image image = io::ImageIO::read(file);
    const pipeline::PipelineResult result = pipeline.process(image);

    if (result.objects.empty()) {
        infra::Logger::warn("Объекты не обнаружены на: " + file.string());
    } else {
        infra::Logger::info(
            "Обнаружено объектов: " + std::to_string(result.objects.size()));
        for (const auto& obj : result.objects) {
            logObject(obj);
        }
    }

    if (opts.saveNoiseFree && !result.denoised.empty()) {
        const auto out = io::makeDerivedPath(file, "noiseFree");
        io::ImageIO::write(result.denoised, out, config.jpegQuality);
        infra::Logger::info("Сохранено (noiseFree): " + out.string());
    }

    if (opts.saveOutline && !result.annotated.empty()) {
        const auto out = io::makeDerivedPath(file, "shapeOutline");
        io::ImageIO::write(result.annotated, out, config.jpegQuality);
        infra::Logger::info("Сохранено (shapeOutline): " + out.string());
    }
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    using namespace visionCore;

    try {
        const CliOptions opts = parseArgs(argc, argv);
        if (opts.showHelp) {
            printUsage(argc > 0 ? argv[0] : "visionCore_app");
            return opts.inputPath.empty() ? EXIT_FAILURE : EXIT_SUCCESS;
        }

        infra::Logger::setLevel(infra::LogLevel::Info);

        infra::AppConfig config;
        auto pipeline = pipeline::PipelineFactory::createDefault(
            config, opts.saveOutline);

        const auto walker = io::FileWalker::defaultImageWalker();
        const auto files  = walker.collect(opts.inputPath);

        if (files.empty()) {
            infra::Logger::warn(
                "Не найдено подходящих файлов: " + opts.inputPath.string());
            return EXIT_FAILURE;
        }

        infra::Logger::info(
            "Файлов к обработке: " + std::to_string(files.size()));

        std::size_t ok = 0;
        std::size_t fail = 0;

        for (const auto& file : files) {
            try {
                infra::Logger::info("--- " + file.string());
                processFile(file, opts, config, *pipeline);
                ++ok;
            } catch (const std::exception& ex) {
                infra::Logger::error(file.string() + ": " + ex.what());
                ++fail;
            }
        }

        infra::Logger::info(
            "Готово. Успешно: " + std::to_string(ok)
            + ", ошибок: " + std::to_string(fail));

        return fail == 0 ? EXIT_SUCCESS : EXIT_FAILURE;

    } catch (const std::exception& ex) {
        infra::Logger::error(std::string("Фатальная ошибка: ") + ex.what());
        printUsage(argc > 0 ? argv[0] : "visionCore_app");
        return EXIT_FAILURE;
    }
}
