#include "visionCore/domain/shapes/Circle.hpp"

#include "ShapeMatchUtils.hpp"

namespace visionCore::domain::shapes {

ShapeKind Circle::kind() const noexcept {
    return ShapeKind::Circle;
}

float Circle::match(const FeatureVector& features) const {
    if (features.vertexCount >= 3 && features.vertexCount <= 5
        && features.circularity < 0.82) {
        return 0.05f;
    }
    const float circ = rangeScore(features.circularity, 0.78, 1.08);
    const float verts = rangeScore(
        static_cast<double>(features.vertexCount), 6.0, 32.0);
    return circ * 0.65f + verts * 0.35f;
}

} // namespace visionCore::domain::shapes
