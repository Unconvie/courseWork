#pragma once

namespace visionCore::domain {

/**
 * @brief Ограничивающий прямоугольник объекта в пиксельных координатах.
 *
 * Система координат совпадает с OpenCV: начало в левом верхнем углу,
 * ось X вправо, ось Y вниз.
 *
 * Поля:
 *  - (x, y) — левый верхний угол bbox;
 *  - (width, height) — размеры в пикселях.
 *
 * Вместе задают область [x, x+width) × [y, y+height).
 */
struct BoundingBox {
    /// X-координата левого верхнего угла (пиксели).
    int x = 0;

    /// Y-координата левого верхнего угла (пиксели).
    int y = 0;

    /// Ширина bbox в пикселях.
    int width = 0;

    /// Высота bbox в пикселях.
    int height = 0;

    /**
     * @brief Проверка, что bbox имеет положительные размеры.
     */
    [[nodiscard]] bool valid() const noexcept {
        return width > 0 && height > 0;
    }
};

/**
 * @brief Точка на изображении в пиксельных координатах.
 */
struct Point2i {
    /// Координата X (пиксели, вправо).
    int x = 0;

    /// Координата Y (пиксели, вниз).
    int y = 0;
};

} // namespace visionCore::domain
