#include "visionCore/pipeline/Pipeline.hpp"

#include <stdexcept>
#include <utility>

namespace visionCore::pipeline {

Pipeline::Pipeline(std::unique_ptr<INoiseFilter> noiseFilter,
                   std::unique_ptr<IDetector> detector,
                   std::unique_ptr<IFeatureExtractor> featureExtractor,
                   std::unique_ptr<IClassifier> classifier,
                   std::unique_ptr<IAnnotator> annotator)
    : m_noiseFilter(std::move(noiseFilter))
    , m_detector(std::move(detector))
    , m_featureExtractor(std::move(featureExtractor))
    , m_classifier(std::move(classifier))
    , m_annotator(std::move(annotator)) {
    if (!m_noiseFilter || !m_detector || !m_featureExtractor || !m_classifier) {
        throw std::invalid_argument(
            "Pipeline: noiseFilter, detector, featureExtractor и classifier "
            "не могут быть nullptr");
    }
}

PipelineResult Pipeline::process(const core::Image& input) {
    if (input.empty()) {
        throw std::invalid_argument("Pipeline::process: пустое изображение");
    }

    PipelineResult result;
    result.original = input;

    result.denoised = m_noiseFilter->filter(input);

    // Детекция на исходнике: тонкие обводки треугольника/многоугольника
    // не теряются из-за размытия noiseFree.
    std::vector<domain::Detection> detections = m_detector->detect(input);

    std::vector<AnnotatedObject> forAnnotator;
    forAnnotator.reserve(detections.size());

    int nextId = 0;
    for (auto& detection : detections) {
        detection.id = nextId++;

        ProcessedObject obj;
        obj.detection = detection;
        obj.features = m_featureExtractor->extract(input, detection);
        obj.classification = m_classifier->classify(obj.features);

        result.objects.push_back(std::move(obj));

        AnnotatedObject ann;
        ann.detection = detection;
        ann.classification = result.objects.back().classification;
        forAnnotator.push_back(std::move(ann));
    }

    if (m_annotator) {
        result.annotated = m_annotator->annotate(input, forAnnotator);
    }

    return result;
}

} // namespace visionCore::pipeline
