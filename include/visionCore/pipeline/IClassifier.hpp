#pragma once

#include "visionCore/domain/Classification.hpp"
#include "visionCore/domain/FeatureVector.hpp"

namespace visionCore::pipeline {

/**
 * @brief Интерфейс классификации по признакам.
 *
 * Реализации:
 *  - ShapeClassifier (Фаза 3) — геометрические формы;
 *  - в будущем AstroClassifier, MLClassifier — без смены Pipeline.
 */
class IClassifier {
public:
    virtual ~IClassifier() = default;

    /**
     * @brief Классифицировать объект по признакам.
     * @param features Признаки, извлечённые IFeatureExtractor.
     * @return Распределение уверенности по классам.
     */
    [[nodiscard]] virtual domain::Classification
        classify(const domain::FeatureVector& features) = 0;
};

} // namespace visionCore::pipeline
