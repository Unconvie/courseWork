#pragma once

#include <vector>

#include "visionCore/core/Image.hpp"
#include "visionCore/domain/Classification.hpp"
#include "visionCore/domain/Detection.hpp"
#include "visionCore/domain/FeatureVector.hpp"

namespace visionCore::pipeline {

/**
 * @brief Полный результат обработки одного кадра через Pipeline.
 */
struct ProcessedObject {
    /// Обнаруженный кандидат (bbox, контур, центроид).
    domain::Detection detection;

    /// Извлечённые признаки (вход для классификатора).
    domain::FeatureVector features;

    /// Распределение уверенности по формам.
    domain::Classification classification;
};

/**
 * @brief Итог работы Pipeline для одного входного изображения.
 */
struct PipelineResult {
    /// Исходный кадр (как был загружен).
    core::Image original;

    /// Кадр после INoiseFilter (может быть пустым, если шаг пропущен).
    core::Image denoised;

    /// Все найденные и классифицированные объекты.
    std::vector<ProcessedObject> objects;

    /// Кадр с разметкой (bbox, контуры, подписи); пустой, если аннотация не вызывалась.
    core::Image annotated;
};

} // namespace visionCore::pipeline
