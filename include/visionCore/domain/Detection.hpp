#pragma once

#include <vector>

#include <opencv2/core.hpp>

#include "visionCore/domain/BoundingBox.hpp"

namespace visionCore::domain {

/**
 * @brief Результат шага обнаружения: один кандидат на изображении.
 *
 * Один кадр может содержать несколько Detection (vector в PipelineResult).
 * Координаты — в пикселях, система как у OpenCV.
 */
struct Detection {
    /// Уникальный индекс кандидата на данном кадре (0, 1, 2, …).
    int id = 0;

    /// Ограничивающий прямоугольник вокруг объекта.
    BoundingBox bbox;

    /**
     * @brief Центроид (центр масс контура).
     *
     * Удобен для отчётов и будущей привязки к небесным координатам
     * через ICoordinateMapper.
     */
    Point2i centroid;

    /// Контур объекта — последовательность точек (пиксели).
    std::vector<cv::Point> contour;

    /// Площадь контура в пикселях².
    double area = 0.0;

    /// Периметр контура в пикселях.
    double perimeter = 0.0;
};

} // namespace visionCore::domain
