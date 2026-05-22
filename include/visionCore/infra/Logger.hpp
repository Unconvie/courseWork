#pragma once

#include <string_view>

namespace visionCore::infra {

/**
 * @brief Уровни логирования.
 *
 * Сообщения с уровнем ниже текущего настроенного через Logger::setLevel
 * отбрасываются на этапе вывода. Это позволяет на этапе разработки крутить
 * DEBUG, а в готовой программе ограничиться INFO/WARN и не захламлять вывод.
 */
enum class LogLevel : int {
    Debug = 0, ///< Подробные сообщения для отладки.
    Info  = 1, ///< Информационные сообщения о ходе работы.
    Warn  = 2, ///< Предупреждения о подозрительных, но не фатальных ситуациях.
    Error = 3  ///< Фатальные для текущей операции ошибки.
};

/**
 * @brief Простой статический логгер.
 *
 * Пишет в stdout сообщения уровня Debug/Info и в stderr - уровня Warn/Error.
 * В файл осознанно не пишет: на этапе разработки это создавало бы хвосты.
 * Когда понадобится файловый бэкенд, логгер будет вынесен за интерфейс
 * ILogger с двумя реализациями (консольной и файловой).
 *
 * Класс статический (фактически namespace, оформленный классом). Состояние
 * - только текущий уровень логирования - общее на всю программу, чего для
 * консольной утилиты достаточно.
 */
class Logger {
public:
    /**
     * @brief Установить минимальный уровень логирования.
     * @param level Новый минимальный уровень. Сообщения ниже отбрасываются.
     */
    static void setLevel(LogLevel level) noexcept;

    /**
     * @brief Получить текущий минимальный уровень логирования.
     */
    [[nodiscard]] static LogLevel level() noexcept;

    /**
     * @brief Записать сообщение указанного уровня.
     * @param level Уровень сообщения.
     * @param message Текст сообщения.
     */
    static void log(LogLevel level, std::string_view message);

    /// Сокращение для @ref log с уровнем Debug.
    static void debug(std::string_view message);
    /// Сокращение для @ref log с уровнем Info.
    static void info(std::string_view message);
    /// Сокращение для @ref log с уровнем Warn.
    static void warn(std::string_view message);
    /// Сокращение для @ref log с уровнем Error.
    static void error(std::string_view message);

    Logger() = delete; ///< Класс используется только статически.

private:
    /// Текущий минимальный уровень логирования (общий для всей программы).
    static LogLevel s_level;
};

} // namespace visionCore::infra
