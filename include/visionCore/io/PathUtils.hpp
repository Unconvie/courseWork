#pragma once

#include <filesystem>
#include <string_view>

namespace visionCore::io {

/**
 * @brief Сформировать путь к производному файлу рядом с исходником.
 *
 * Пример: photo.jpg + "-noiseFree" -> photo-noiseFree.jpg
 *
 * @param source Исходный путь к файлу.
 * @param suffix Суффикс без дефиса в начале (например "noiseFree").
 */
[[nodiscard]] std::filesystem::path makeDerivedPath(
    const std::filesystem::path& source,
    std::string_view suffix);

/** @brief true, если файл — результат pipeline (noiseFree / shapeOutline). */
[[nodiscard]] bool isDerivedOutput(const std::filesystem::path& path);

} // namespace visionCore::io
