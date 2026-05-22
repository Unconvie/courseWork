#pragma once

#include <memory>
#include <vector>

#include "visionCore/domain/Shape.hpp"
#include "visionCore/infra/AppConfig.hpp"
#include "visionCore/pipeline/IClassifier.hpp"

namespace visionCore::pipeline::impl {

/**
 * @brief Классификатор геометрических форм по эталонам Shape.
 */
class ShapeClassifier : public IClassifier {
public:
    explicit ShapeClassifier(infra::AppConfig config);

    [[nodiscard]] domain::Classification
        classify(const domain::FeatureVector& features) override;

private:
    infra::AppConfig m_config;

    /// Зарегистрированные эталоны форм (Circle, Triangle, …).
    std::vector<std::unique_ptr<domain::Shape>> m_shapes;
};

} // namespace visionCore::pipeline::impl
