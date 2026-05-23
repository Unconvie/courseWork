#pragma once

namespace visionCore::app {

/** @brief Какие производные файлы сохранять после обработки. */
struct OutputOptions {
    bool saveNoiseFree = false;
    bool saveOutline = true;
    bool saveReport = true;
};

}  // namespace visionCore::app
