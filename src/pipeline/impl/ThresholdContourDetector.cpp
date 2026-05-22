#include "visionCore/pipeline/impl/ThresholdContourDetector.hpp"

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>

namespace visionCore::pipeline::impl {

namespace {

double rectIoU(const cv::Rect& a, const cv::Rect& b) {
    const int x1 = std::max(a.x, b.x);
    const int y1 = std::max(a.y, b.y);
    const int x2 = std::min(a.x + a.width, b.x + b.width);
    const int y2 = std::min(a.y + a.height, b.y + b.height);
    if (x2 <= x1 || y2 <= y1) {
        return 0.0;
    }
    const double inter = static_cast<double>((x2 - x1) * (y2 - y1));
    const double uni =
        static_cast<double>(a.area() + b.area()) - inter;
    return uni > 1e-6 ? inter / uni : 0.0;
}

bool isFrameContour(const cv::Rect& rect,
                    const cv::Size& imageSize,
                    double area,
                    double maxAreaRatio) {
    const double frameArea =
        static_cast<double>(imageSize.width * imageSize.height);
    if (area > frameArea * maxAreaRatio) {
        return true;
    }
    const int margin = 2;
    const bool touchesBorder =
        rect.x <= margin || rect.y <= margin
        || rect.x + rect.width >= imageSize.width - margin
        || rect.y + rect.height >= imageSize.height - margin;
    return touchesBorder && area > frameArea * 0.35;
}

domain::Detection makeDetection(const std::vector<cv::Point>& contour,
                               int id) {
    const double area = cv::contourArea(contour);
    const cv::Rect rect = cv::boundingRect(contour);
    const cv::Moments mu = cv::moments(contour);
    const double perimeter = cv::arcLength(contour, true);

    domain::Detection det;
    det.id = id;
    det.bbox = {rect.x, rect.y, rect.width, rect.height};
    if (mu.m00 > 1e-6) {
        det.centroid.x = static_cast<int>(mu.m10 / mu.m00);
        det.centroid.y = static_cast<int>(mu.m01 / mu.m00);
    } else {
        det.centroid = {rect.x + rect.width / 2, rect.y + rect.height / 2};
    }
    det.contour = contour;
    det.area = area;
    det.perimeter = perimeter;
    return det;
}

} // anonymous namespace

ThresholdContourDetector::ThresholdContourDetector(infra::AppConfig config)
    : m_config(std::move(config)) {}

std::vector<domain::Detection>
ThresholdContourDetector::detect(const core::Image& image) {
    cv::Mat gray;
    if (image.channels() == 1) {
        gray = image.data();
    } else {
        cv::cvtColor(image.data(), gray, cv::COLOR_BGR2GRAY);
    }

    cv::Mat work = gray;
    if (m_config.detectGaussianSigma > 0.0) {
        const int ksize =
            static_cast<int>(m_config.detectGaussianSigma * 6) | 1;
        cv::GaussianBlur(gray, work, cv::Size(ksize, ksize),
                         m_config.detectGaussianSigma);
    }

    const cv::Size imageSize = work.size();

    std::vector<domain::Detection> candidates;

    auto tryAdd = [&](const std::vector<cv::Point>& contour,
                      double minArea, bool fromCanny) {
        const double area = cv::contourArea(contour);
        if (area < minArea) {
            return;
        }
        const cv::Rect rect = cv::boundingRect(contour);
        const int shortSide = std::min(rect.width, rect.height);
        const int longSide = std::max(rect.width, rect.height);
        if (shortSide < m_config.minBboxSide) {
            return;
        }
        if (longSide > 0
            && static_cast<double>(longSide) / shortSide > 12.0
            && area < 1200.0) {
            return;
        }
        if (fromCanny && rect.width * rect.height > 0) {
            const double fill = area / (rect.width * rect.height);
            if (fill < 0.08) {
                return;
            }
        }
        if (isFrameContour(rect, imageSize, area,
                           m_config.maxContourAreaRatio)) {
            return;
        }
        candidates.push_back(
            makeDetection(contour, static_cast<int>(candidates.size())));
    };

    // --- Залитые области и вложенные контуры (RETR_TREE) ---
    cv::Mat binary;
    if (m_config.adaptiveThreshold) {
        cv::adaptiveThreshold(work, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                              cv::THRESH_BINARY_INV, 15, 4);
    } else {
        cv::threshold(work, binary, m_config.thresholdValue, 255,
                      cv::THRESH_BINARY_INV);
    }

    cv::Mat opened;
    cv::morphologyEx(binary, opened, cv::MORPH_OPEN,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, {3, 3}));

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(opened, contours, hierarchy, cv::RETR_TREE,
                     cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        tryAdd(contour, m_config.minContourArea, false);
    }

    // --- Тонкие обводки (Canny + замыкание линий) ---
    if (m_config.useCannyEdges) {
        cv::Mat edges;
        cv::Canny(work, edges, m_config.cannyLow, m_config.cannyHigh);
        cv::morphologyEx(
            edges, edges, cv::MORPH_CLOSE,
            cv::getStructuringElement(cv::MORPH_ELLIPSE, {5, 5}));

        std::vector<std::vector<cv::Point>> edgeContours;
        cv::findContours(edges, edgeContours, cv::RETR_LIST,
                         cv::CHAIN_APPROX_SIMPLE);
        for (const auto& contour : edgeContours) {
            tryAdd(contour, m_config.minCannyContourArea, true);
        }
    }

    // --- Удаление дубликатов (перекрывающиеся bbox) ---
    std::vector<domain::Detection> result;
    for (auto& cand : candidates) {
        bool duplicate = false;
        for (std::size_t i = 0; i < result.size(); ++i) {
            const cv::Rect a(cand.bbox.x, cand.bbox.y, cand.bbox.width,
                             cand.bbox.height);
            const cv::Rect b(result[i].bbox.x, result[i].bbox.y,
                             result[i].bbox.width, result[i].bbox.height);
            if (rectIoU(a, b) > m_config.contourDedupeIou) {
                duplicate = true;
                if (cand.area > result[i].area) {
                    result[i] = std::move(cand);
                }
                break;
            }
        }
        if (!duplicate) {
            result.push_back(std::move(cand));
        }
    }

    // Стабильные id после слияния
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i].id = static_cast<int>(i);
    }

    return result;
}

} // namespace visionCore::pipeline::impl
