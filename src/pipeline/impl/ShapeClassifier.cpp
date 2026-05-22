#include "visionCore/pipeline/impl/ShapeClassifier.hpp"

#include <algorithm>
#include <numeric>

#include "visionCore/domain/shapes/Circle.hpp"
#include "visionCore/domain/shapes/Hexagon.hpp"
#include "visionCore/domain/shapes/Rectangle.hpp"
#include "visionCore/domain/shapes/Square.hpp"
#include "visionCore/domain/shapes/Triangle.hpp"

namespace visionCore::pipeline::impl {

ShapeClassifier::ShapeClassifier(infra::AppConfig config)
    : m_config(std::move(config)) {
    m_shapes.push_back(std::make_unique<domain::shapes::Circle>());
    m_shapes.push_back(std::make_unique<domain::shapes::Triangle>());
    m_shapes.push_back(std::make_unique<domain::shapes::Rectangle>());
    m_shapes.push_back(std::make_unique<domain::shapes::Square>());
    m_shapes.push_back(std::make_unique<domain::shapes::Hexagon>());
}

domain::Classification
ShapeClassifier::classify(const domain::FeatureVector& features) {
    domain::Classification result;
    float sum = 0.0f;

    for (const auto& shape : m_shapes) {
        const float score = shape->match(features);
        if (score > 0.0f) {
            result.scores[shape->kind()] = score;
            sum += score;
        }
    }

    if (sum > 1e-6f) {
        for (auto& [kind, score] : result.scores) {
            score /= sum;
        }
    }

    if (result.topConfidence() < m_config.minClassificationConfidence) {
        result.scores.clear();
        result.scores[domain::ShapeKind::Unknown] = 1.0f;
    }

    return result;
}

} // namespace visionCore::pipeline::impl
