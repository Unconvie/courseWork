#include "visionCore/domain/shapes/Hexagon.hpp"

#include "ShapeMatchUtils.hpp"

namespace visionCore::domain::shapes {

ShapeKind Hexagon::kind() const noexcept {
    return ShapeKind::Hexagon;
}

float Hexagon::match(const FeatureVector& features) const {
    const float verts =
        targetScore(static_cast<double>(features.vertexCount), 6.0, 1.2);
    const float lowCirc =
        rangeScore(features.circularity, 0.0, 0.88);
    return verts * 0.75f + lowCirc * 0.25f;
}

} // namespace visionCore::domain::shapes
