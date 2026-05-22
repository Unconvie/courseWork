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

bool centroidInside(const domain::Detection& inner,
                    const domain::Detection& outer) {
    const cv::Point c(inner.centroid.x, inner.centroid.y);
    const cv::Rect box(outer.bbox.x, outer.bbox.y, outer.bbox.width,
                       outer.bbox.height);
    return box.contains(c);
}

std::vector<cv::Point> circleContour(int cx, int cy, int radius) {
    std::vector<cv::Point> pts;
    constexpr int kSegments = 32;
    pts.reserve(kSegments);
    for (int i = 0; i < kSegments; ++i) {
        const double angle = (2.0 * CV_PI * i) / kSegments;
        pts.emplace_back(
            cx + static_cast<int>(radius * std::cos(angle)),
            cy + static_cast<int>(radius * std::sin(angle)));
    }
    return pts;
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

void suppressNested(std::vector<domain::Detection>& detections,
                    double maxAreaRatio) {
    std::vector<bool> drop(detections.size(), false);
    for (std::size_t i = 0; i < detections.size(); ++i) {
        for (std::size_t j = 0; j < detections.size(); ++j) {
            if (i == j || drop[i]) {
                continue;
            }
            if (detections[i].area >= detections[j].area) {
                continue;
            }
            if (detections[i].area > detections[j].area * maxAreaRatio) {
                continue;
            }
            if (centroidInside(detections[i], detections[j])) {
                drop[i] = true;
                break;
            }
            const cv::Rect a(detections[i].bbox.x, detections[i].bbox.y,
                             detections[i].bbox.width, detections[i].bbox.height);
            const cv::Rect b(detections[j].bbox.x, detections[j].bbox.y,
                             detections[j].bbox.width, detections[j].bbox.height);
            if (rectIoU(a, b) > 0.35 && detections[i].area < detections[j].area * 0.5) {
                drop[i] = true;
                break;
            }
        }
    }

    std::vector<domain::Detection> kept;
    kept.reserve(detections.size());
    for (std::size_t i = 0; i < detections.size(); ++i) {
        if (!drop[i]) {
            kept.push_back(std::move(detections[i]));
        }
    }
    detections = std::move(kept);
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

    cv::Mat enhanced = work;
    if (m_config.useClahe) {
        const auto clahe =
            cv::createCLAHE(m_config.claheClipLimit, {8, 8});
        clahe->apply(work, enhanced);
    }

    const cv::Size imageSize = enhanced.size();
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

    // --- Залитые области: только внешние контуры (без «квадратов» внутри круга) ---
    cv::Mat binary;
    if (m_config.adaptiveThreshold) {
        cv::adaptiveThreshold(enhanced, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                              cv::THRESH_BINARY_INV, 15, 4);
    } else {
        cv::threshold(enhanced, binary, m_config.thresholdValue, 255,
                      cv::THRESH_BINARY_INV);
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    for (const auto& contour : contours) {
        tryAdd(contour, m_config.minContourArea, false);
    }

    auto runCannyPass = [&](int low, int high, double minArea, int closeSize) {
        cv::Mat edges;
        cv::Canny(enhanced, edges, low, high);
        cv::morphologyEx(
            edges, edges, cv::MORPH_CLOSE,
            cv::getStructuringElement(cv::MORPH_ELLIPSE, {closeSize, closeSize}));

        std::vector<std::vector<cv::Point>> edgeContours;
        cv::findContours(edges, edgeContours, cv::RETR_LIST,
                         cv::CHAIN_APPROX_SIMPLE);
        for (const auto& contour : edgeContours) {
            tryAdd(contour, minArea, true);
        }
    };

    if (m_config.useCannyEdges) {
        runCannyPass(m_config.cannyLow, m_config.cannyHigh,
                     m_config.minCannyContourArea, 5);
        runCannyPass(m_config.faintCannyLow, m_config.faintCannyHigh,
                     m_config.minFaintContourArea, 3);
    }

    // --- Отдельные залитые круги (в т.ч. два пересекающихся) ---
    if (m_config.useHoughCircles) {
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(enhanced, circles, cv::HOUGH_GRADIENT,
                         m_config.houghDp, m_config.houghMinDist,
                         m_config.houghParam1, m_config.houghParam2,
                         m_config.houghMinRadius, m_config.houghMaxRadius);
        for (const auto& c : circles) {
            const int cx = cvRound(c[0]);
            const int cy = cvRound(c[1]);
            const int r  = cvRound(c[2]);
            if (r < m_config.houghMinRadius) {
                continue;
            }
            const auto contour = circleContour(cx, cy, r);
            tryAdd(contour, CV_PI * r * r * 0.5, false);
        }
    }

    suppressNested(candidates, m_config.nestedMaxAreaRatio);

    // --- Слияние дубликатов по IoU ---
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

    suppressNested(result, m_config.nestedMaxAreaRatio);

    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i].id = static_cast<int>(i);
    }

    return result;
}

} // namespace visionCore::pipeline::impl
