#pragma once

#include "visionCore/domain/Shape.hpp"

namespace visionCore::domain::shapes {

/**
 * @brief Эталон формы «квадрат» (4 вершины, aspect ratio ≈ 1).
 */
class Square : public Shape {
public:
    [[nodiscard]] ShapeKind kind() const noexcept override;

    [[nodiscard]] float match(const FeatureVector& features) const override;
};

} // namespace visionCore::domain::shapes
