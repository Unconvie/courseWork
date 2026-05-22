#pragma once

#include "visionCore/domain/Shape.hpp"

namespace visionCore::domain::shapes {

/**
 * @brief Эталон формы «круг».
 *
 * Опирается на высокую circularity (≈ 1) и большое число вершин
 * аппроксимации (круг аппроксимируется многогранником).
 */
class Circle : public Shape {
public:
    [[nodiscard]] ShapeKind kind() const noexcept override;

    [[nodiscard]] float match(const FeatureVector& features) const override;
};

} // namespace visionCore::domain::shapes
