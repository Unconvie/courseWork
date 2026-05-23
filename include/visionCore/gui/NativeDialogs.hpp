#pragma once

#include <string>

namespace visionCore::gui {

/** @brief Диалог выбора одного файла изображения. false = отмена. */
bool pickImageFile(std::string& outPathUtf8);

/** @brief Диалог выбора папки. false = отмена. */
bool pickFolder(std::string& outPathUtf8);

}  // namespace visionCore::gui
