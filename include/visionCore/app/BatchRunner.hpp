#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "visionCore/app/OutputOptions.hpp"
#include "visionCore/infra/AppConfig.hpp"

namespace visionCore::app {

struct BatchRequest {
    std::filesystem::path inputPath;
    infra::AppConfig config;
    OutputOptions outputs;
};

struct BatchResult {
    std::size_t filesTotal = 0;
    std::size_t ok = 0;
    std::size_t fail = 0;
    bool success = false;
    std::string summary;
    std::vector<std::string> logLines;
};

/**
 * @brief Обработать файл или все исходники в папке.
 * @param request Путь, конфиг, флаги вывода.
 * @return Статистика и строки лога (для GUI).
 */
[[nodiscard]] BatchResult runBatch(const BatchRequest& request);

}  // namespace visionCore::app
