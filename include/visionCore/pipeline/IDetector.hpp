#pragma once

#include <vector>

#include "visionCore/core/Image.hpp"
#include "visionCore/domain/Detection.hpp"

namespace visionCore::pipeline {

/**
 * @brief Интерфейс шага обнаружения объектов на изображении.
 *
 * Возвращает ноль или больше кандидатов (Detection). Реализации (Фаза 3):
 * ThresholdContourDetector, EdgeContourDetector и т.д.
 */
class IDetector {
public:
    virtual ~IDetector() = default;

    /**
     * @brief Найти кандидатов на изображении.
     *
     * @param image Изображение для анализа (часто уже после INoiseFilter).
     *        Детектор сам решает, использовать grayscale или цвет.
     * @return Список обнаружений; может быть пустым.
     */
    [[nodiscard]] virtual std::vector<domain::Detection>
        detect(const core::Image& image) = 0;
};

} // namespace visionCore::pipeline
