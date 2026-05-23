#include "visionCore/io/FileWalker.hpp"

#include "visionCore/io/PathUtils.hpp"

#include <algorithm>
#include <cctype>
#include <system_error>
#include <utility>

namespace visionCore::io {

FileWalker::FileWalker(std::unordered_set<std::string> extensions,
                       bool recursive)
    : m_extensions(std::move(extensions)), m_recursive(recursive) {}

// Какие расширения считаются «картинкой» при обходе папки.
FileWalker FileWalker::defaultImageWalker() {
  return FileWalker({".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff"});
}

bool FileWalker::isExtensionSupported(
    const std::filesystem::path& path) const {
  const auto ext = toLower(path.extension().string());
  return m_extensions.find(ext) != m_extensions.end();
}

// КАКОЙ ФАЙЛ ОБРАБАТЫВАЕТСЯ: один файл или все подходящие в каталоге.
std::vector<std::filesystem::path>
FileWalker::collect(const std::filesystem::path& path) const {
  std::vector<std::filesystem::path> result;
  std::error_code ec;

  if (!std::filesystem::exists(path, ec) || ec) {
    return result;
  }

  if (std::filesystem::is_regular_file(path, ec) && !ec) {
    if (isExtensionSupported(path) && !isDerivedOutput(path)) {
      result.push_back(path);
    }
    return result;
  }

  if (!std::filesystem::is_directory(path, ec) || ec) {
    return result;
  }

  auto append = [&](const std::filesystem::directory_entry& entry) {
    std::error_code inner;
    if (entry.is_regular_file(inner) && !inner &&
        isExtensionSupported(entry.path()) && !isDerivedOutput(entry.path())) {
      result.push_back(entry.path());
    }
  };

  if (m_recursive) {
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(path, ec)) {
      append(entry);
    }
  } else {
    for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
      append(entry);
    }
  }

  std::sort(result.begin(), result.end());
  return result;
}

std::string FileWalker::toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

}  // namespace visionCore::io
