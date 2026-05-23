#include "visionCore/domain/shapes/Circle.hpp"

#include "ShapeMatchUtils.hpp"

namespace visionCore::domain::shapes {

ShapeKind Circle::kind() const noexcept {
  return ShapeKind::Circle;
}

// ═══════════════════════════════════════════════════════════════════════════
//  ЭТАЛОН «КРУГ»: правила match(), не обучение.
//  Чем ближе признаки к идеалу — тем выше score. См. FeatureVector.hpp.
//  Аналогичные файлы: Triangle.cpp, Square.cpp, Hexagon.cpp, …
// ═══════════════════════════════════════════════════════════════════════════
float Circle::match(const FeatureVector& features) const {
  // Явный треугольник/четырёхугольник — не круг.
  if (features.vertexCount >= 3 && features.vertexCount <= 5
      && features.circularity < 0.82) {
    return 0.05f;
  }
  // Шести-/семиугольник с умеренной circularity — отдаём Hexagon.
  if (features.vertexCount >= 6 && features.vertexCount <= 9
      && features.circularity < 0.92) {
    return 0.08f;
  }
  const float circ = rangeScore(features.circularity, 0.82, 1.08);
  const float verts =
      rangeScore(static_cast<double>(features.vertexCount), 8.0, 32.0);
  return circ * 0.7f + verts * 0.3f;
}

}  // namespace visionCore::domain::shapes
