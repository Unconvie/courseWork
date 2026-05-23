#include "visionCore/app/BatchRunner.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "visionCore/core/Image.hpp"
#include "visionCore/domain/ShapeKind.hpp"
#include "visionCore/infra/Logger.hpp"
#include "visionCore/io/FileWalker.hpp"
#include "visionCore/io/ImageIO.hpp"
#include "visionCore/io/PathUtils.hpp"
#include "visionCore/pipeline/PipelineFactory.hpp"
#include "visionCore/pipeline/PipelineResult.hpp"

namespace visionCore::app {

namespace {

void appendLog(BatchResult& result, const std::string& line,
               infra::LogLevel level = infra::LogLevel::Info) {
    result.logLines.push_back(line);
    switch (level) {
        case infra::LogLevel::Warn:
            infra::Logger::warn(line);
            break;
        case infra::LogLevel::Error:
            infra::Logger::error(line);
            break;
        default:
            infra::Logger::info(line);
            break;
    }
}

std::string formatObjectBlock(const pipeline::ProcessedObject& obj) {
    const auto& d = obj.detection;
    const auto kind = obj.classification.topShape();
    const float conf = obj.classification.topConfidence();

    std::ostringstream oss;
    oss << "  Объект #" << d.id << ": " << domain::toString(kind) << " ("
        << std::fixed << std::setprecision(0) << (conf * 100.0f) << "%)\n"
        << "    bbox: (x=" << d.bbox.x << ", y=" << d.bbox.y
        << ", w=" << d.bbox.width << ", h=" << d.bbox.height << ")\n"
        << "    центроид: (" << d.centroid.x << ", " << d.centroid.y << ")\n"
        << "    площадь: " << static_cast<int>(d.area) << " px^2";

    if (obj.classification.scores.size() > 1) {
        oss << "\n    распределение:";
        for (const auto& [k, v] : obj.classification.scores) {
            oss << " " << domain::toString(k) << "=" << std::fixed
                << std::setprecision(0) << (v * 100.0f) << "%";
        }
    }
    return oss.str();
}

std::string buildReportText(const std::filesystem::path& file,
                            const pipeline::PipelineResult& result) {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif

    std::ostringstream oss;
    oss << "visionCore — отчёт обработки\n"
        << "Файл: " << file.u8string() << "\n"
        << "Время: " << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "\n"
        << "Обнаружено объектов: " << result.objects.size() << "\n\n";

    if (result.objects.empty()) {
        oss << "Объекты не обнаружены.\n";
    } else {
        for (const auto& obj : result.objects) {
            oss << formatObjectBlock(obj) << "\n\n";
        }
    }
    return oss.str();
}

void writeReportFile(const std::filesystem::path& source,
                     const std::string& text,
                     BatchResult& batchLog) {
    const auto path = io::makeReportPath(source);
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Не удалось создать отчёт: " + path.u8string());
    }
    out.write("\xEF\xBB\xBF", 3);
    out << text;
    appendLog(batchLog, "Сохранено (report): " + path.u8string());
}

void processFile(const std::filesystem::path& file,
                 const BatchRequest& request,
                 pipeline::Pipeline& pipeline,
                 BatchResult& batchLog) {
    const core::Image image = io::ImageIO::read(file);
    const pipeline::PipelineResult result = pipeline.process(image);

    if (result.objects.empty()) {
        appendLog(batchLog,
                  "Объекты не обнаружены на: " + file.u8string(),
                  infra::LogLevel::Warn);
    } else {
        appendLog(batchLog,
                  "Обнаружено объектов: "
                      + std::to_string(result.objects.size()));
        for (const auto& obj : result.objects) {
            appendLog(batchLog, formatObjectBlock(obj));
        }
    }

    if (request.outputs.saveReport) {
        writeReportFile(file, buildReportText(file, result), batchLog);
    }

    if (request.outputs.saveNoiseFree && !result.denoised.empty()) {
        const auto out = io::makeDerivedPath(file, "noiseFree");
        io::ImageIO::write(result.denoised, out, request.config.jpegQuality);
        appendLog(batchLog, "Сохранено (noiseFree): " + out.u8string());
    }

    if (request.outputs.saveOutline && !result.annotated.empty()) {
        const auto out = io::makeDerivedPath(file, "shapeOutline");
        io::ImageIO::write(result.annotated, out, request.config.jpegQuality);
        appendLog(batchLog, "Сохранено (shapeOutline): " + out.u8string());
    }
}

}  // anonymous namespace

BatchResult runBatch(const BatchRequest& request) {
    BatchResult result;

    if (request.inputPath.empty()) {
        result.summary = "Не указан путь к файлу или папке.";
        appendLog(result, result.summary, infra::LogLevel::Error);
        return result;
    }

    auto pipeline = pipeline::PipelineFactory::createDefault(
        request.config, request.outputs.saveOutline);

    const auto walker = io::FileWalker::defaultImageWalker();
    const auto files = walker.collect(request.inputPath);

    if (files.empty()) {
        result.summary =
            "Не найдено исходных изображений (без -noiseFree, -shapeOutline, -copy).";
        appendLog(result, result.summary, infra::LogLevel::Warn);
        return result;
    }

    result.filesTotal = files.size();
    appendLog(result,
              "Файлов к обработке: " + std::to_string(files.size()));

    for (const auto& file : files) {
        try {
            appendLog(result, "--- " + file.u8string());
            processFile(file, request, *pipeline, result);
            ++result.ok;
        } catch (const std::exception& ex) {
            appendLog(result, file.u8string() + ": " + ex.what(),
                      infra::LogLevel::Error);
            ++result.fail;
        }
    }

    result.success = (result.fail == 0);
    result.summary = "Готово. Успешно: " + std::to_string(result.ok)
        + ", ошибок: " + std::to_string(result.fail);
    appendLog(result, result.summary);

    return result;
}

}  // namespace visionCore::app
