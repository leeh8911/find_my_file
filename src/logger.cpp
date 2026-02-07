/**
 * @file logger.cpp
 * @brief Implementation of Logger class
 */

#include "logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace fmf
{

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger() : minLevel_(LogLevel::INFO), consoleOutput_(true) {}

Logger::~Logger() { close(); }

void Logger::setLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(mutex_);
    minLevel_ = level;
}

LogLevel Logger::getLevel() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return minLevel_;
}

bool Logger::setLogFile(const std::string& filepath)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Close existing file if open
    if (logFile_.is_open())
    {
        logFile_.close();
    }

    // If filepath is empty, just close the file
    if (filepath.empty())
    {
        return true;
    }

    // Try to open new file
    logFile_.open(filepath, std::ios::app);
    return logFile_.is_open();
}

void Logger::setConsoleOutput(bool enable)
{
    std::lock_guard<std::mutex> lock(mutex_);
    consoleOutput_ = enable;
}

bool Logger::isConsoleOutputEnabled() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return consoleOutput_;
}

void Logger::debug(const std::string& message)
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) { log(LogLevel::INFO, message); }

void Logger::warn(const std::string& message) { log(LogLevel::WARN, message); }

void Logger::error(const std::string& message)
{
    log(LogLevel::ERROR, message);
}

void Logger::flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open())
    {
        logFile_.flush();
    }
    if (consoleOutput_)
    {
        std::cout.flush();
        std::cerr.flush();
    }
}

void Logger::close()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open())
    {
        logFile_.close();
    }
}

void Logger::log(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Filter by log level
    if (level < minLevel_)
    {
        return;
    }

    // Build log message
    std::ostringstream oss;
    oss << "[" << getCurrentTimestamp() << "] [" << levelToString(level) << "] "
        << message << "\n";

    std::string logMessage = oss.str();

    // Output to console
    if (consoleOutput_)
    {
        if (level >= LogLevel::ERROR)
        {
            std::cerr << logMessage;
        }
        else
        {
            std::cout << logMessage;
        }
    }

    // Output to file
    if (logFile_.is_open())
    {
        logFile_ << logMessage;
    }
}

std::string Logger::levelToString(LogLevel level) const
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::NONE:
            return "NONE";
        default:
            return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch())
                      .count() %
                  1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << now_ms;

    return oss.str();
}

}  // namespace fmf
