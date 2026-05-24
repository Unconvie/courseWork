#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace visionCore::tools {

/** Параметры генерации набора тестовых изображений. */
struct SampleGeneratorOptions {
    /** Каталог вывода (по умолчанию data/samples). */
    std::filesystem::path outputDir = "data/samples";
    int width = 800;
    int height = 600;
    /** Фиксированное зерно — одинаковый набор при повторном запуске. */
    unsigned seed = 42;
};

/** Результат генерации. */
struct SampleGeneratorResult {
    int filesWritten = 0;
    std::vector<std::filesystem::path> paths;
    std::string summary;
};

/**
 * @brief Создаёт набор PNG с геометрическими фигурами и разными типами шума.
 *
 * Имена файлов начинаются с gen-*, чтобы не путать с пользовательскими тестами.
 */
[[nodiscard]] SampleGeneratorResult generateSampleImages(
    const SampleGeneratorOptions& options = {});

}  // namespace visionCore::tools
