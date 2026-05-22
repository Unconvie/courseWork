#pragma once

#include "visionCore/infra/AppConfig.hpp"
#include "visionCore/pipeline/IDetector.hpp"

namespace visionCore::pipeline::impl {

/**
 * @brief Обнаружение: бинаризация (RETR_TREE) + контуры Canny, слияние дубликатов.
 */
class ThresholdContourDetector : public IDetector {
public:
    explicit ThresholdContourDetector(infra::AppConfig config);

    [[nodiscard]] std::vector<domain::Detection>
        detect(const core::Image& image) override;

private:
    infra::AppConfig m_config;
};

} // namespace visionCore::pipeline::impl
