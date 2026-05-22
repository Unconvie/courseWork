#pragma once

#include "visionCore/core/Image.hpp"

namespace visionCore::pipeline {

/**
 * @brief Интерфейс шага предобработки: подавление шума.
 *
 * Реализации (Фаза 3): GaussianMedianFilter и др.
 * Замена реализации не требует изменений в Pipeline и Application.
 */
class INoiseFilter {
public:
    virtual ~INoiseFilter() = default;

    /**
     * @brief Отфильтровать шум на изображении.
     * @param input Исходное изображение (может быть цветным).
     * @return Изображение после фильтрации; sourcePath копируется из input.
     */
    [[nodiscard]] virtual core::Image filter(const core::Image& input) = 0;
};

} // namespace visionCore::pipeline
