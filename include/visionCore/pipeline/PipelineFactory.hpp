#pragma once

#include <memory>

#include "visionCore/infra/AppConfig.hpp"
#include "visionCore/pipeline/Pipeline.hpp"

namespace visionCore::pipeline {

/**
 * @brief Фабрика сборки pipeline с реализациями по умолчанию (Фаза 3).
 */
class PipelineFactory {
public:
    /**
     * @brief Создать pipeline со стандартными реализациями шагов.
     * @param config Параметры алгоритмов.
     * @param withAnnotator Если true — создаётся IAnnotator для -shapeOutline.
     */
    [[nodiscard]] static std::unique_ptr<Pipeline>
        createDefault(const infra::AppConfig& config, bool withAnnotator);
};

} // namespace visionCore::pipeline
