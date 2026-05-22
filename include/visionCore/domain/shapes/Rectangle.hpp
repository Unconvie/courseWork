#pragma once

#include "visionCore/domain/Shape.hpp"

namespace visionCore::domain::shapes {

/**
 * @brief Эталон формы «прямоугольник» (4 вершины, aspect ratio ≠ 1).
 */
class Rectangle : public Shape {
public:
    [[nodiscard]] ShapeKind kind() const noexcept override;

    [[nodiscard]] float match(const FeatureVector& features) const override;
};

} // namespace visionCore::domain::shapes
