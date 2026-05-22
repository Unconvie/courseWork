#include "visionCore/infra/Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace visionCore::infra {

LogLevel Logger::s_level = LogLevel::Info;

void Logger::setLevel(LogLevel level) noexcept {
    s_level = level;
}

LogLevel Logger::level() noexcept {
    return s_level;
}

namespace {

/**
 * @brief Получить текстовый тег уровня лога фиксированной ширины.
 *
 * Ширина зафиксирована (5 символов), чтобы все строки лога визуально
 * выравнивались в консоли независимо от уровня.
 */
std::string_view levelTag(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
    }
    return "?????";
}

/**
 * @brief Текущее локальное время в формате HH:MM:SS.
 *
 * Используется для префикса каждой строки лога. Дату не пишем намеренно -
 * лишний шум для короткоживущих запусков курсовой утилиты.
 */
std::string timestamp() {
    using namespace std::chrono;
    const auto now = system_clock::to_time_t(system_clock::now());

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");
    return ss.str();
}

} // anonymous namespace

void Logger::log(LogLevel level, std::string_view message) {
    if (static_cast<int>(level) < static_cast<int>(s_level)) {
        return;
    }

    auto& out = (level == LogLevel::Warn || level == LogLevel::Error)
        ? std::cerr
        : std::cout;

    out << '[' << timestamp() << "] ["
        << levelTag(level) << "] "
        << message << '\n';
}

void Logger::debug(std::string_view message) { log(LogLevel::Debug, message); }
void Logger::info (std::string_view message) { log(LogLevel::Info,  message); }
void Logger::warn (std::string_view message) { log(LogLevel::Warn,  message); }
void Logger::error(std::string_view message) { log(LogLevel::Error, message); }

} // namespace visionCore::infra
