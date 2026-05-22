#include "visionCore/domain/ShapeKind.hpp"

namespace visionCore::domain {

std::string_view toString(ShapeKind kind) noexcept {
    switch (kind) {
        case ShapeKind::Circle:    return "Circle";
        case ShapeKind::Triangle:  return "Triangle";
        case ShapeKind::Rectangle: return "Rectangle";
        case ShapeKind::Square:    return "Square";
        case ShapeKind::Hexagon:   return "Hexagon";
        case ShapeKind::Unknown:
        default:                   return "Unknown";
    }
}

} // namespace visionCore::domain
