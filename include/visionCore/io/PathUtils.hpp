#pragma once

#include <filesystem>
#include <string_view>

namespace visionCore::io {

/**
 * @brief Путь к производному файлу рядом с исходником.
 *
 * КУДА СОХРАНЯЮТСЯ КОПИИ (сейчас):
 *   исходник:  D:/data/тест1.png
 *   noiseFree: D:/data/тест1-noiseFree.png
 *   outline:   D:/data/тест1-shapeOutline.png
 *
 * Отдельной папки вывода нет — только parent_path(source).
 * Чтобы писать в другую директорию, меняй реализацию в PathUtils.cpp
 * или логику в main.cpp::processFile().
 *
 * @param suffix Без дефиса: "noiseFree", "shapeOutline".
 */
[[nodiscard]] std::filesystem::path makeDerivedPath(
    const std::filesystem::path& source,
    std::string_view suffix);

/**
 * @brief Пропуск при обходе папки (FileWalker).
 *
 * Иначе повторный запуск по data/samples обработает уже созданные
 * noiseFree/shapeOutline и получит цепочки имён *-noiseFree-noiseFree.
 */
/**
 * @brief true, если файл — результат обработки (составное имя), а не исходник.
 *
 * Такие файлы FileWalker не берёт в работу: *-noiseFree, *-shapeOutline, *-copy.
 */
[[nodiscard]] bool isDerivedOutput(const std::filesystem::path& path);

/**
 * @brief Путь к текстовому отчёту рядом с исходником.
 *
 * Пример: photo.png → photo-report.txt
 */
[[nodiscard]] std::filesystem::path makeReportPath(
    const std::filesystem::path& source);

}  // namespace visionCore::io
