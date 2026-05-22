#include "visionCore/domain/shapes/Triangle.hpp"

#include "ShapeMatchUtils.hpp"

namespace visionCore::domain::shapes {

ShapeKind Triangle::kind() const noexcept {
    return ShapeKind::Triangle;
}

float Triangle::match(const FeatureVector& features) const {
    const float verts =
        targetScore(static_cast<double>(features.vertexCount), 3.0, 0.75);
    const float lowCirc =
        rangeScore(features.circularity, 0.0, 0.78);
    return verts * 0.65f + lowCirc * 0.35f;
}

} // namespace visionCore::domain::shapes
