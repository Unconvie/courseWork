#pragma once

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

/**
 * @brief Включить UTF-8 в консоли Windows (русский текст в логах).
 *
 * Вызывать один раз в начале main(). Вместе с /utf-8 в CMakeLists.txt.
 */
namespace visionCore::infra {

inline void setupConsoleUtf8() {
#if defined(_WIN32)
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
}

}  // namespace visionCore::infra
