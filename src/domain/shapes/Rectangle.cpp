#include "visionCore/domain/shapes/Rectangle.hpp"

#include "ShapeMatchUtils.hpp"

namespace visionCore::domain::shapes {

ShapeKind Rectangle::kind() const noexcept {
    return ShapeKind::Rectangle;
}

float Rectangle::match(const FeatureVector& features) const {
    const float verts = targetScore(
        static_cast<double>(features.vertexCount), 4.0, 0.5);
    const float aspect = rangeScore(features.aspectRatio, 1.2, 5.0);
    const float lowCirc =
        rangeScore(features.circularity, 0.0, 0.68);
    return (verts + aspect + lowCirc) / 3.0f;
}

} // namespace visionCore::domain::shapes
