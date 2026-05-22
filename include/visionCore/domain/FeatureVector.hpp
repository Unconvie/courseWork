#pragma once

#include <vector>

namespace visionCore::domain {

/**
 * @brief Числовые признаки контура объекта для классификации формы.
 *
 * Заполняется шагом IFeatureExtractor (Фаза 3). Классы Shape сравнивают
 * эти значения со своими эталонами. Отделение признаков от Detection
 * позволяет:
 *  - переиспользовать один FeatureVector разными классификаторами;
 *  - позже подключить AstroClassifier с другим набором признаков.
 *
 * Поля рассчитаны на геометрическую классификацию (курсовая). Для ML-
 * классификатора можно расширить вектор, не меняя Detection.
 */
struct FeatureVector {
    /// Число вершин после аппроксимации контура (Дуглас–Пёкер).
    int vertexCount = 0;

    /**
     * @brief Circulariry: 4π·area / perimeter².
     *
     * Для идеального круга ≈ 1.0; для вытянутых фигур меньше.
     */
    double circularity = 0.0;

    /**
     * @brief Отношение сторон bbox: width / height.
     *
     * Для квадрата ≈ 1.0; для прямоугольника отличается от 1.
     */
    double aspectRatio = 1.0;

    /// Площадь контура в пикселях² (для фильтрации и отчётов).
    double area = 0.0;

    /// Периметр контура в пикселях.
    double perimeter = 0.0;

    /**
     * @brief Hu-моменты (опционально, до 7 значений).
     *
     * Инвариантны к масштабу и повороту; заполняются в Фазе 3.
     * Пустой вектор — классификатор опирается только на vertexCount,
     * circularity и aspectRatio.
     */
    std::vector<double> huMoments;
};

} // namespace visionCore::domain
