#pragma once

#include "visionCore/domain/FeatureVector.hpp"
#include "visionCore/domain/ShapeKind.hpp"

namespace visionCore::domain {

/**
 * @brief Абстрактный эталон геометрической формы.
 *
 * Каждая конкретная форма (Circle, Triangle, …) хранит критерии «насколько
 * признаки похожи на этот класс» и реализует match(). ShapeClassifier
 * (Фаза 3) обходит все зарегистрированные Shape и собирает scores.
 *
 * Не путать с астрономическими типами объектов — для них будет отдельный
 * классификатор без иерархии Shape.
 */
class Shape {
public:
    virtual ~Shape() = default;

    /**
     * @brief Тип формы, которую представляет этот класс.
     */
    [[nodiscard]] virtual ShapeKind kind() const noexcept = 0;

    /**
     * @brief Оценить похожесть признаков на эту форму.
     * @param features Признаки, извлечённые из контура.
     * @return Уверенность в диапазоне [0, 1].
     */
    [[nodiscard]] virtual float match(const FeatureVector& features) const = 0;
};

} // namespace visionCore::domain
