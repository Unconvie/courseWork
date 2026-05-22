#include "visionCore/domain/shapes/Hexagon.hpp"

#include "ShapeMatchUtils.hpp"

namespace visionCore::domain::shapes {

ShapeKind Hexagon::kind() const noexcept {
    return ShapeKind::Hexagon;
}

float Hexagon::match(const FeatureVector& features) const {
    const float verts =
        rangeScore(static_cast<double>(features.vertexCount), 5.0, 9.0);
    const float lowCirc =
        rangeScore(features.circularity, 0.0, 0.82);
    return verts * 0.7f + lowCirc * 0.3f;
}

} // namespace visionCore::domain::shapes
