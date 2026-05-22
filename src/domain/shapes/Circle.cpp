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
    // Шести-/многоугольник с ~6–9 вершинами не должен побеждать круг.
    if (features.vertexCount >= 6 && features.vertexCount <= 9
        && features.circularity < 0.92) {
        return 0.08f;
    }
    const float circ = rangeScore(features.circularity, 0.82, 1.08);
    const float verts = rangeScore(
        static_cast<double>(features.vertexCount), 8.0, 32.0);
    return circ * 0.7f + verts * 0.3f;
}

} // namespace visionCore::domain::shapes
