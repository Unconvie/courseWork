#pragma once

#include <vector>

#include "visionCore/core/Image.hpp"
#include "visionCore/domain/Classification.hpp"
#include "visionCore/domain/Detection.hpp"

namespace visionCore::pipeline {

/**
 * @brief Пара «обнаружение + классификация» для одного объекта на кадре.
 */
struct AnnotatedObject {
    /// Геометрия и контур кандидата.
    domain::Detection detection;

    /// Результат классификации формы.
    domain::Classification classification;
};

/**
 * @brief Интерфейс визуальной разметки результатов на изображении.
 *
 * Рисует bbox, контур и подпись с формой и уверенностью. Реализация
 * OpenCvAnnotator — в Фазе 3.
 */
class IAnnotator {
public:
    virtual ~IAnnotator() = default;

    /**
     * @brief Нарисовать разметку поверх копии исходного кадра.
     *
     * @param original Исходное изображение (цвет сохраняется).
     * @param objects Список объектов для отрисовки.
     * @return Новое изображение с разметкой; sourcePath как у original.
     */
    [[nodiscard]] virtual core::Image annotate(
        const core::Image& original,
        const std::vector<AnnotatedObject>& objects) = 0;
};

} // namespace visionCore::pipeline
