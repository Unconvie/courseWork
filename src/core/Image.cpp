#include "visionCore/core/Image.hpp"

#include <utility>

namespace visionCore::core {

Image::Image(cv::Mat data, std::filesystem::path sourcePath)
    : m_data(std::move(data))
    , m_sourcePath(std::move(sourcePath)) {}

std::string Image::stem() const {
    return m_sourcePath.stem().string();
}

std::string Image::extension() const {
    return m_sourcePath.extension().string();
}

} // namespace visionCore::core
