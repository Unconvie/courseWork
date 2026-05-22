#include "visionCore/io/PathUtils.hpp"

#include <string>

namespace visionCore::io {

std::filesystem::path makeDerivedPath(const std::filesystem::path& source,
                                      std::string_view suffix) {
    const auto folder = source.parent_path();
    const auto stem   = source.stem().string();
    const auto ext    = source.extension().string();
    return folder / (stem + "-" + std::string(suffix) + ext);
}

bool isDerivedOutput(const std::filesystem::path& path) {
    const auto stem = path.stem().string();
    return stem.find("-noiseFree") != std::string::npos
        || stem.find("-shapeOutline") != std::string::npos;
}

} // namespace visionCore::io
