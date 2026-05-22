#include "visionCore/pipeline/impl/OpenCvAnnotator.hpp"

#include <cstdio>
#include <string>

#include <opencv2/imgproc.hpp>

#include "visionCore/domain/ShapeKind.hpp"

namespace visionCore::pipeline::impl {

namespace {

cv::Scalar colorFor(visionCore::domain::ShapeKind kind) {
    // BGR: на белом фоне хорошо видны насыщенные тёмные/контрастные цвета.
    switch (kind) {
        case visionCore::domain::ShapeKind::Circle:
            return {220, 60, 20};    // синий
        case visionCore::domain::ShapeKind::Triangle:
            return {180, 0, 180};    // фиолетовый
        case visionCore::domain::ShapeKind::Rectangle:
            return {0, 120, 255};    // оранжевый
        case visionCore::domain::ShapeKind::Square:
            return {0, 160, 0};      // зелёный
        case visionCore::domain::ShapeKind::Hexagon:
            return {40, 40, 220};    // красный
        default:
            return {90, 90, 90};
    }
}

void drawLabel(cv::Mat& canvas, const std::string& text, cv::Point origin,
               const cv::Scalar& color) {
    const int y = std::max(14, origin.y);
    const cv::Point pos(origin.x, y);
    cv::putText(canvas, text, pos, cv::FONT_HERSHEY_SIMPLEX, 0.55,
                cv::Scalar(0, 0, 0), 4, cv::LINE_AA);
    cv::putText(canvas, text, pos, cv::FONT_HERSHEY_SIMPLEX, 0.55,
                color, 2, cv::LINE_AA);
}

} // anonymous namespace

core::Image OpenCvAnnotator::annotate(
    const core::Image& original,
    const std::vector<AnnotatedObject>& objects) {
    cv::Mat canvas;
    if (original.channels() == 1) {
        cv::cvtColor(original.data(), canvas, cv::COLOR_GRAY2BGR);
    } else {
        canvas = original.data().clone();
    }

    constexpr int kThickness = 3;

    for (const auto& obj : objects) {
        const auto kind = obj.classification.topShape();
        const cv::Scalar color = colorFor(kind);

        if (!obj.detection.contour.empty()) {
            std::vector<std::vector<cv::Point>> contours{obj.detection.contour};
            cv::drawContours(canvas, contours, -1, color, kThickness,
                             cv::LINE_AA);
        }

        const auto& b = obj.detection.bbox;
        cv::rectangle(canvas,
                      cv::Rect(b.x, b.y, b.width, b.height),
                      color, kThickness, cv::LINE_AA);

        char label[128];
        std::snprintf(label, sizeof(label), "%s %.0f%%",
                      std::string(domain::toString(kind)).c_str(),
                      obj.classification.topConfidence() * 100.0f);

        drawLabel(canvas, label, cv::Point(b.x, b.y - 4), color);
    }

    return core::Image{canvas, original.sourcePath()};
}

} // namespace visionCore::pipeline::impl
