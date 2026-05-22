#pragma once

#include "visionCore/domain/Shape.hpp"

namespace visionCore::domain::shapes {

/**
 * @brief Эталон формы «шестиугольник» (6 вершин аппроксимации).
 */
class Hexagon : public Shape {
public:
    [[nodiscard]] ShapeKind kind() const noexcept override;

    [[nodiscard]] float match(const FeatureVector& features) const override;
};

} // namespace visionCore::domain::shapes
