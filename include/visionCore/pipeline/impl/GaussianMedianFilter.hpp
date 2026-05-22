#pragma once

#include "visionCore/infra/AppConfig.hpp"
#include "visionCore/pipeline/INoiseFilter.hpp"

namespace visionCore::pipeline::impl {

/**
 * @brief Подавление шума: Gaussian blur + median blur (OpenCV).
 */
class GaussianMedianFilter : public INoiseFilter {
public:
    explicit GaussianMedianFilter(infra::AppConfig config);

    [[nodiscard]] core::Image filter(const core::Image& input) override;

private:
    infra::AppConfig m_config;
};

} // namespace visionCore::pipeline::impl
