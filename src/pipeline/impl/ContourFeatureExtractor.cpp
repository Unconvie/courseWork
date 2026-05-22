#include "visionCore/pipeline/impl/ContourFeatureExtractor.hpp"

#include <cmath>

#include <opencv2/imgproc.hpp>

namespace visionCore::pipeline::impl {

ContourFeatureExtractor::ContourFeatureExtractor(infra::AppConfig config)
    : m_config(std::move(config)) {}

domain::FeatureVector ContourFeatureExtractor::extract(
    const core::Image& /*image*/,
    const domain::Detection& detection) {
    domain::FeatureVector features;
    features.area = detection.area;
    features.perimeter = detection.perimeter;

    if (detection.bbox.width > 0 && detection.bbox.height > 0) {
        features.aspectRatio =
            static_cast<double>(detection.bbox.width) /
            static_cast<double>(detection.bbox.height);
    }

    if (detection.perimeter > 1e-6) {
        features.circularity =
            (4.0 * CV_PI * detection.area) /
            (detection.perimeter * detection.perimeter);
    }

    if (!detection.contour.empty()) {
        std::vector<cv::Point> approx;
        const double eps =
            m_config.contourApproxEpsilon * detection.perimeter;
        cv::approxPolyDP(detection.contour, approx, eps, true);
        features.vertexCount = static_cast<int>(approx.size());
    }

    return features;
}

} // namespace visionCore::pipeline::impl
