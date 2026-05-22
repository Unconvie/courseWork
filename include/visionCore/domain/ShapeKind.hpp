#pragma once

#include <string_view>

namespace visionCore::domain {

/**
 * @brief Перечисление поддерживаемых геометрических форм.
 *
 * Используется классификатором форм (ShapeClassifier в Фазе 3) и классами-
 * наследниками Shape. Значение Unknown означает «не удалось уверенно
 * отнести к известному классу».
 *
 * Для астрономической классификации в будущем будет отдельный enum
 * (AstroObjectKind) и отдельный IClassifier — не расширяем этот enum
 * «космическими» типами.
 */
enum class ShapeKind {
    Unknown   = 0, ///< Форма не определена или уверенность ниже порога.
    Circle    = 1, ///< Окружность / эллипс с высокой circularity.
    Triangle  = 2, ///< Треугольник (3 вершины аппроксимации).
    Rectangle = 3, ///< Прямоугольник (4 вершины, стороны не равны).
    Square    = 4, ///< Квадрат (4 вершины, aspect ratio ≈ 1).
    Hexagon   = 5  ///< Шестиугольник (6 вершин аппроксимации).
};

/**
 * @brief Человекочитаемое имя формы для логов и разметки на изображении.
 * @param kind Тип формы.
 * @return Строковый литерал (не владеет памятью).
 */
[[nodiscard]] std::string_view toString(ShapeKind kind) noexcept;

} // namespace visionCore::domain
