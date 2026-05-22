#include "visionCore/domain/shapes/Square.hpp"

#include "ShapeMatchUtils.hpp"

namespace visionCore::domain::shapes {

ShapeKind Square::kind() const noexcept {
    return ShapeKind::Square;
}

float Square::match(const FeatureVector& features) const {
    const float verts = rangeScore(
        static_cast<double>(features.vertexCount), 4.0, 10.0);
    const float aspect = rangeScore(features.aspectRatio, 0.75, 1.25);
    const float lowCirc =
        rangeScore(features.circularity, 0.0, 0.72);
    return (verts + aspect + lowCirc) / 3.0f;
}

} // namespace visionCore::domain::shapes
