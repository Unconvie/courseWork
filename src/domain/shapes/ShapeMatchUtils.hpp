#pragma once

#include <algorithm>
#include <cmath>

namespace visionCore::domain::shapes {

/**
 * @brief Степень попадания значения в диапазон [low, high].
 *
 * @param value Проверяемое значение.
 * @param low Нижняя граница (включительно).
 * @param high Верхняя граница (включительно).
 * @return 1.0 внутри диапазона; плавное затухание снаружи.
 */
inline float rangeScore(double value, double low, double high) {
    if (value >= low && value <= high) {
        return 1.0f;
    }
    const double dist = (value < low) ? (low - value) : (value - high);
    const double span = std::max(high - low, 1.0);
    return static_cast<float>(std::max(0.0, 1.0 - dist / span));
}

/**
 * @brief Степень близости value к target (чем ближе, тем выше).
 *
 * @param value Фактическое значение.
 * @param target Целевое значение.
 * @param tolerance Допустимое отклонение для score = 1.
 */
inline float targetScore(double value, double target, double tolerance) {
    const double dist = std::abs(value - target);
    if (dist <= tolerance) {
        return 1.0f;
    }
    return static_cast<float>(std::max(0.0, 1.0 - dist / (tolerance * 2.0)));
}

} // namespace visionCore::domain::shapes
