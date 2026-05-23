#include "visionCore/io/PathUtils.hpp"

#include <string>

namespace visionCore::io {

std::filesystem::path makeDerivedPath(const std::filesystem::path& source,
                                      std::string_view suffix) {
  // Папка вывода = папка исходного файла (не cwd программы).
  const auto folder = source.parent_path();
  const auto stem = source.stem().string();
  const auto ext = source.extension().string();
  // Итог: <stem>-<suffix><ext>
  return folder / (stem + "-" + std::string(suffix) + ext);
}

bool isDerivedOutput(const std::filesystem::path& path) {
  const auto stem = path.stem().string();
  return stem.find("-noiseFree") != std::string::npos
      || stem.find("-shapeOutline") != std::string::npos
      || stem.size() >= 5
          && stem.compare(stem.size() - 5, 5, "-copy") == 0;
}

std::filesystem::path makeReportPath(const std::filesystem::path& source) {
  return source.parent_path() / (source.stem().string() + "-report.txt");
}

}  // namespace visionCore::io
