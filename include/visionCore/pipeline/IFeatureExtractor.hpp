#pragma once

#include "visionCore/core/Image.hpp"
#include "visionCore/domain/Detection.hpp"
#include "visionCore/domain/FeatureVector.hpp"

namespace visionCore::pipeline {

/**
 * @brief Интерфейс извлечения признаков из контура кандидата.
 *
 * Отделён от IDetector, чтобы одни и те же Detection могли
 * обрабатываться разными классификаторами (форма сейчас, астрономия потом).
 */
class IFeatureExtractor {
public:
    virtual ~IFeatureExtractor() = default;

    /**
     * @brief Извлечь признаки для одного кандидата.
     *
     * @param image Контекст кадра (для доступа к пикселям внутри контура).
     * @param detection Обнаружение с заполненным contour и bbox.
     * @return Вектор признаков для передачи в IClassifier.
     */
    [[nodiscard]] virtual domain::FeatureVector extract(
        const core::Image& image,
        const domain::Detection& detection) = 0;
};

} // namespace visionCore::pipeline
