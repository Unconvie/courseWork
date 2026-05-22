#pragma once

#include <memory>

#include "visionCore/core/Image.hpp"
#include "visionCore/pipeline/IAnnotator.hpp"
#include "visionCore/pipeline/IClassifier.hpp"
#include "visionCore/pipeline/IDetector.hpp"
#include "visionCore/pipeline/IFeatureExtractor.hpp"
#include "visionCore/pipeline/INoiseFilter.hpp"
#include "visionCore/pipeline/PipelineResult.hpp"

namespace visionCore::pipeline {

/**
 * @brief Оркестратор цепочки обработки изображения.
 *
 * Принимает реализации шагов через интерфейсы (внедрение зависимостей).
 * Порядок шагов фиксирован:
 *   1. INoiseFilter
 *   2. IDetector
 *   3. IFeatureExtractor (для каждого кандидата)
 *   4. IClassifier (для каждого кандидата)
 *   5. IAnnotator (если передан не nullptr)
 *
 * Замена любого шага (например, на оптимизированную или ML-версию)
 * делается при сборке Application без правок тела Pipeline.
 */
class Pipeline {
public:
    /**
     * @brief Собрать pipeline из конкретных реализаций шагов.
     *
     * @param noiseFilter Фильтрация шума (не nullptr).
     * @param detector Обнаружение кандидатов (не nullptr).
     * @param featureExtractor Извлечение признаков (не nullptr).
     * @param classifier Классификация формы (не nullptr).
     * @param annotator Разметка на изображении; может быть nullptr — тогда
     *        annotated в результате останется пустым.
     */
    Pipeline(std::unique_ptr<INoiseFilter> noiseFilter,
               std::unique_ptr<IDetector> detector,
               std::unique_ptr<IFeatureExtractor> featureExtractor,
               std::unique_ptr<IClassifier> classifier,
               std::unique_ptr<IAnnotator> annotator = nullptr);

    /**
     * @brief Обработать одно изображение всей цепочкой.
     * @param input Загруженное изображение с заполненным sourcePath.
     * @return Структура с промежуточными и итоговыми данными.
     */
    [[nodiscard]] PipelineResult process(const core::Image& input);

private:
    /// Шаг подавления шума.
    std::unique_ptr<INoiseFilter> m_noiseFilter;

    /// Шаг обнаружения кандидатов.
    std::unique_ptr<IDetector> m_detector;

    /// Шаг извлечения признаков.
    std::unique_ptr<IFeatureExtractor> m_featureExtractor;

    /// Шаг классификации.
    std::unique_ptr<IClassifier> m_classifier;

    /// Шаг визуальной разметки (опционально).
    std::unique_ptr<IAnnotator> m_annotator;
};

} // namespace visionCore::pipeline
