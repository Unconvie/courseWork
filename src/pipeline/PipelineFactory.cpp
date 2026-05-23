#include "visionCore/pipeline/PipelineFactory.hpp"

#include "visionCore/pipeline/impl/ContourFeatureExtractor.hpp"
#include "visionCore/pipeline/impl/GaussianMedianFilter.hpp"
#include "visionCore/pipeline/impl/OpenCvAnnotator.hpp"
#include "visionCore/pipeline/impl/ShapeClassifier.hpp"
#include "visionCore/pipeline/impl/ThresholdContourDetector.hpp"

namespace visionCore::pipeline {

// Сборка «цепочки по умолчанию». Чтобы заменить алгоритм — подставь другой
// класс, реализующий тот же интерфейс (INoiseFilter, IDetector, …).
std::unique_ptr<Pipeline> PipelineFactory::createDefault(
    const infra::AppConfig& config,
    bool withAnnotator) {
  std::unique_ptr<IAnnotator> annotator;
  if (withAnnotator) {
    annotator = std::make_unique<impl::OpenCvAnnotator>();
  }

  return std::make_unique<Pipeline>(
      std::make_unique<impl::GaussianMedianFilter>(config),   // *-noiseFree
      std::make_unique<impl::ThresholdContourDetector>(config),  // контуры
      std::make_unique<impl::ContourFeatureExtractor>(config),   // признаки
      std::make_unique<impl::ShapeClassifier>(config),           // форма
      std::move(annotator));  // подписи на *-shapeOutline
}

}  // namespace visionCore::pipeline
