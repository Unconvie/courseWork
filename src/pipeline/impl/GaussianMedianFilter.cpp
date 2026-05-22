#include "visionCore/pipeline/impl/GaussianMedianFilter.hpp"

#include <opencv2/imgproc.hpp>

namespace visionCore::pipeline::impl {

GaussianMedianFilter::GaussianMedianFilter(infra::AppConfig config)
    : m_config(std::move(config)) {}

core::Image GaussianMedianFilter::filter(const core::Image& input) {
    // Для цветных снимков: лёгкий bilateral — меньше «мыла», чем gray+blur.
    if (input.channels() >= 3 && m_config.bilateralDiameter >= 3) {
        cv::Mat out = input.data().clone();
        const int d = m_config.bilateralDiameter;
        cv::bilateralFilter(input.data(), out, d, 50.0, 50.0);
        return core::Image{out, input.sourcePath()};
    }

    cv::Mat gray;
    if (input.channels() == 1) {
        gray = input.data().clone();
    } else {
        cv::cvtColor(input.data(), gray, cv::COLOR_BGR2GRAY);
    }

    cv::Mat filtered = gray;

    if (m_config.gaussianSigma > 0.0) {
        const int ksize = static_cast<int>(m_config.gaussianSigma * 6) | 1;
        cv::GaussianBlur(filtered, filtered, cv::Size(ksize, ksize),
                         m_config.gaussianSigma);
    }

    if (m_config.medianKernel >= 3) {
        int k = m_config.medianKernel;
        if (k % 2 == 0) {
            ++k;
        }
        cv::medianBlur(filtered, filtered, k);
    }

    cv::Mat bgr;
    cv::cvtColor(filtered, bgr, cv::COLOR_GRAY2BGR);

    return core::Image{bgr, input.sourcePath()};
}

} // namespace visionCore::pipeline::impl
