#pragma once

#include "visionCore/infra/AppConfig.hpp"
#include "visionCore/pipeline/IFeatureExtractor.hpp"

namespace visionCore::pipeline::impl {

/**
 * @brief Извлечение геометрических признаков из контура (аппроксимация, circularity).
 */
class ContourFeatureExtractor : public IFeatureExtractor {
public:
    explicit ContourFeatureExtractor(infra::AppConfig config);

    [[nodiscard]] domain::FeatureVector extract(
        const core::Image& image,
        const domain::Detection& detection) override;

private:
    infra::AppConfig m_config;
};

} // namespace visionCore::pipeline::impl
