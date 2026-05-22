#pragma once

#include <map>

#include "visionCore/domain/ShapeKind.hpp"

namespace visionCore::domain {

/**
 * @brief Результат классификации формы: распределение уверенности по классам.
 *
 * scores[Circle] = 0.8 означает «80% похоже на круг». Сумма не обязана
 * быть ровно 1.0 до нормализации; ShapeClassifier нормализует при выдаче.
 */
struct Classification {
    /// Уверенность по каждому известному классу формы [0..1].
    std::map<ShapeKind, float> scores;

    /**
     * @brief Класс с максимальной уверенностью.
     * @return ShapeKind::Unknown, если scores пуст.
     */
    [[nodiscard]] ShapeKind topShape() const noexcept;

    /**
     * @brief Уверенность для topShape().
     * @return 0.0f, если scores пуст.
     */
    [[nodiscard]] float topConfidence() const noexcept;
};

} // namespace visionCore::domain
