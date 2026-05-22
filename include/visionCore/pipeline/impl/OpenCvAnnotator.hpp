#pragma once

#include "visionCore/pipeline/IAnnotator.hpp"

namespace visionCore::pipeline::impl {

/**
 * @brief Рисует bbox, контур и подпись формы с уверенностью (OpenCV).
 */
class OpenCvAnnotator : public IAnnotator {
public:
    [[nodiscard]] core::Image annotate(
        const core::Image& original,
        const std::vector<AnnotatedObject>& objects) override;
};

} // namespace visionCore::pipeline::impl
