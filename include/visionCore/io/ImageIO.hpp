#pragma once

#include <filesystem>
#include <stdexcept>

#include "visionCore/core/Image.hpp"

namespace visionCore::io {

/**
 * @brief Исключение, выбрасываемое функциями ImageIO при ошибках чтения/записи.
 *
 * Содержит читаемое сообщение с путём к файлу. Ловится на верхнем уровне
 * (в main или в Pipeline), чтобы одна битая картинка не валила всю
 * batch-обработку - просто конкретный файл пропускается и логируется.
 */
class ImageIoError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Чтение и запись изображений с диска.
 *
 * Тонкая обёртка над cv::imread / cv::imwrite, которая:
 *  - возвращает доменный тип visionCore::core::Image вместо голого cv::Mat,
 *  - конвертирует «тихие» провалы OpenCV (пустая матрица) в исключения,
 *  - умеет проверять предусловия (существование файла и т.п.).
 *
 * Класс полностью статический и сейчас оформлен именно классом (а не
 * пространством имён) с расчётом на будущую замену на интерфейс IImageIO -
 * когда понадобятся альтернативные реализации, например, для FITS-формата
 * из астрономии или для in-memory источников при тестировании.
 */
class ImageIO {
public:
    /**
     * @brief Загрузить изображение с диска.
     * @param path Путь к файлу.
     * @return Заполненный Image с пиксельными данными и метаданными о пути.
     * @throws ImageIoError если файл не существует, не является обычным
     *         файлом, или OpenCV не смог его распознать как изображение.
     *
     * Используется флаг IMREAD_UNCHANGED, чтобы не терять альфа-канал и
     * не приводить всё к 8-бит/BGR молча - решение, что делать с количеством
     * каналов и глубиной, принимают следующие шаги pipeline, а не I/O.
     */
    static core::Image read(const std::filesystem::path& path);

    /**
     * @brief Сохранить изображение в файл.
     * @param image Изображение для сохранения. Должно быть непустым.
     * @param path Путь для сохранения. Формат определяется по расширению.
     * @param jpegQuality Качество JPEG [0..100]; игнорируется для PNG/BMP.
     * @throws ImageIoError если изображение пустое или запись не удалась.
     *
     * Если родительская папка пути не существует - она создаётся.
     */
    static void write(const core::Image& image,
                      const std::filesystem::path& path,
                      int jpegQuality = 90);

    ImageIO() = delete; ///< Класс используется только статически.
};

} // namespace visionCore::io
