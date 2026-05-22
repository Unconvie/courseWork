#include "visionCore/domain/Classification.hpp"

#include <algorithm>

namespace visionCore::domain {

ShapeKind Classification::topShape() const noexcept {
    if (scores.empty()) {
        return ShapeKind::Unknown;
    }

    const auto it = std::max_element(
        scores.begin(), scores.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    return it->first;
}

float Classification::topConfidence() const noexcept {
    if (scores.empty()) {
        return 0.0f;
    }

    const auto it = std::max_element(
        scores.begin(), scores.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    return it->second;
}

} // namespace visionCore::domain
