// visionCore — один exe: GUI (без аргументов / --gui) или CLI (путь + флаги).

#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

#include "visionCore/app/BatchRunner.hpp"
#include "visionCore/app/OutputOptions.hpp"
#include "visionCore/infra/AppConfig.hpp"
#include "visionCore/infra/ConsoleUtf8.hpp"
#include "visionCore/infra/Logger.hpp"
#include "visionCore/gui/GuiApp.hpp"

namespace {

using namespace visionCore;

struct CliOptions {
    std::filesystem::path inputPath;
    app::OutputOptions outputs;
    bool showHelp = false;
    bool launchGui = false;
};

void printUsage(const char* programName) {
    std::cout
        << "Использование:\n"
        << "  " << programName << "              — графический интерфейс\n"
        << "  " << programName << " --gui         — графический интерфейс\n"
        << "  " << programName << " <путь> [опции] — консольный режим\n"
        << "\n"
        << "Опции CLI:\n"
        << "  --save-noise-free    *-noiseFree\n"
        << "  --save-outline       *-shapeOutline (по умолчанию)\n"
        << "  --no-outline\n"
        << "  --save-report        *-report.txt\n"
        << "  --no-report\n"
        << "  -h, --help\n";
}

CliOptions parseArgs(int argc, char* argv[], const infra::AppConfig& config) {
    CliOptions opts;
    opts.outputs.saveReport = config.saveReportByDefault;

    if (argc < 2) {
        opts.launchGui = true;
        return opts;
    }

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            opts.showHelp = true;
        } else if (arg == "--gui") {
            opts.launchGui = true;
        } else if (arg == "--save-noise-free") {
            opts.outputs.saveNoiseFree = true;
        } else if (arg == "--save-outline") {
            opts.outputs.saveOutline = true;
        } else if (arg == "--no-outline") {
            opts.outputs.saveOutline = false;
        } else if (arg == "--save-report") {
            opts.outputs.saveReport = true;
        } else if (arg == "--no-report") {
            opts.outputs.saveReport = false;
        } else if (arg.front() == '-') {
            throw std::runtime_error("Неизвестный аргумент: " + arg);
        } else if (opts.inputPath.empty()) {
            opts.inputPath = arg;
        } else {
            throw std::runtime_error("Указано несколько путей: " + arg);
        }
    }

    return opts;
}

}  // namespace

int main(int argc, char* argv[]) {
    using namespace visionCore;

    try {
        infra::AppConfig config;
        const CliOptions opts = parseArgs(argc, argv, config);

        if (opts.launchGui) {
            return gui::runGuiApp();
        }

        infra::setupConsoleUtf8();

        if (opts.showHelp) {
            printUsage(argc > 0 ? argv[0] : "visionCore_app");
            return opts.inputPath.empty() ? EXIT_FAILURE : EXIT_SUCCESS;
        }

        infra::Logger::setLevel(infra::LogLevel::Info);

        app::BatchRequest request;
        request.inputPath = opts.inputPath;
        request.config = config;
        request.outputs = opts.outputs;

        const app::BatchResult result = app::runBatch(request);
        return result.success ? EXIT_SUCCESS : EXIT_FAILURE;

    } catch (const std::exception& ex) {
        infra::setupConsoleUtf8();
        infra::Logger::error(std::string("Фатальная ошибка: ") + ex.what());
        printUsage(argc > 0 ? argv[0] : "visionCore_app");
        return EXIT_FAILURE;
    }
}
