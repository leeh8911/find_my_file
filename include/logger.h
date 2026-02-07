/**
 * @file logger.h
 * @brief Logging system for find_my_files application
 *
 * Provides a flexible logging system with multiple log levels and output targets.
 * Follows the Single Responsibility Principle (SRP) by only handling logging concerns.
 *
 * Features:
 * - Multiple log levels (DEBUG, INFO, WARN, ERROR)
 * - Thread-safe logging
 * - Multiple output targets (console, file, both)
 * - Configurable log format
 * - Singleton pattern for global access
 *
 * SOLID Principles:
 * - SRP: Only handles logging, no other responsibilities
 * - OCP: Extensible for new log levels or output formats
 * - DIP: Other components depend on Logger abstraction
 */

#ifndef FMF_LOGGER_H
#define FMF_LOGGER_H

#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace fmf
{

/**
 * @brief Log level enumeration
 *
 * Defines severity levels for log messages.
 * Higher levels include all lower levels.
 */
enum class LogLevel
{
    DEBUG = 0,  ///< Detailed debug information
    INFO = 1,   ///< General informational messages
    WARN = 2,   ///< Warning messages
    ERROR = 3,  ///< Error messages
    NONE = 4    ///< Disable all logging
};

/**
 * @brief Logging system with thread-safe operations
 *
 * Singleton class that provides application-wide logging functionality.
 * Thread-safe for concurrent logging from multiple threads.
 *
 * Example usage:
 * @code
 *   // Initialize logger
 *   Logger::instance().setLevel(LogLevel::INFO);
 *   Logger::instance().setLogFile("app.log");
 *
 *   // Log messages
 *   Logger::instance().debug("Starting application");
 *   Logger::instance().info("Processing file: {}", filename);
 *   Logger::instance().warn("File not found: {}", path);
 *   Logger::instance().error("Failed to open file: {}", error);
 * @endcode
 *
 * @note Thread-safe for all public methods
 * @note Uses mutex for synchronization
 * @note Singleton pattern ensures single global instance
 */
class Logger
{
 public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the Logger instance
     */
    static Logger& instance();

    // Delete copy and move constructors/operators (Singleton)
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    /**
     * @brief Set the minimum log level
     * @param level Minimum level to log (messages below this are ignored)
     */
    void setLevel(LogLevel level);

    /**
     * @brief Get the current log level
     * @return Current log level
     */
    LogLevel getLevel() const;

    /**
     * @brief Set log file path
     * @param filepath Path to log file (empty string to disable file logging)
     * @return true if file opened successfully, false otherwise
     */
    bool setLogFile(const std::string& filepath);

    /**
     * @brief Enable/disable console output
     * @param enable true to enable console logging
     */
    void setConsoleOutput(bool enable);

    /**
     * @brief Check if console output is enabled
     * @return true if console output is enabled
     */
    bool isConsoleOutputEnabled() const;

    /**
     * @brief Log debug message
     * @param message Message to log
     */
    void debug(const std::string& message);

    /**
     * @brief Log info message
     * @param message Message to log
     */
    void info(const std::string& message);

    /**
     * @brief Log warning message
     * @param message Message to log
     */
    void warn(const std::string& message);

    /**
     * @brief Log error message
     * @param message Message to log
     */
    void error(const std::string& message);

    /**
     * @brief Flush all output buffers
     */
    void flush();

    /**
     * @brief Close log file and cleanup
     */
    void close();

 private:
    /**
     * @brief Private constructor (Singleton pattern)
     */
    Logger();

    /**
     * @brief Destructor - closes log file
     */
    ~Logger();

    /**
     * @brief Internal log method
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Get string representation of log level
     * @param level Log level
     * @return String representation (e.g., "INFO", "ERROR")
     */
    std::string levelToString(LogLevel level) const;

    /**
     * @brief Get current timestamp string
     * @return Formatted timestamp string
     */
    std::string getCurrentTimestamp() const;

    LogLevel minLevel_ = LogLevel::INFO;  ///< Minimum log level
    bool consoleOutput_ = true;           ///< Console output enabled
    std::ofstream logFile_;               ///< Log file stream
    mutable std::mutex mutex_;            ///< Mutex for thread safety
};

}  // namespace fmf

#endif  // FMF_LOGGER_H
