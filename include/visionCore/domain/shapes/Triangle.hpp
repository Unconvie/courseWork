#pragma once

#include "visionCore/domain/Shape.hpp"

namespace visionCore::domain::shapes {

/**
 * @brief Эталон формы «треугольник» (3 вершины аппроксимации).
 */
class Triangle : public Shape {
public:
    [[nodiscard]] ShapeKind kind() const noexcept override;

    [[nodiscard]] float match(const FeatureVector& features) const override;
};

} // namespace visionCore::domain::shapes
