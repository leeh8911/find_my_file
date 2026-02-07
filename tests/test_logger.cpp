/**
 * @file test_logger.cpp
 * @brief Unit tests for Logger class
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <thread>

#include "logger.h"

using namespace fmf;

class LoggerTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        // Reset logger to default state
        Logger::instance().setLevel(LogLevel::INFO);
        Logger::instance().setConsoleOutput(true);
        Logger::instance().close();

        // Clean up any test log files
        if (std::filesystem::exists(testLogFile_))
        {
            std::filesystem::remove(testLogFile_);
        }
    }

    void TearDown() override
    {
        Logger::instance().close();
        if (std::filesystem::exists(testLogFile_))
        {
            std::filesystem::remove(testLogFile_);
        }
    }

    std::string testLogFile_ = "test_logger.log";

    std::string readLogFile(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    int countLinesInFile(const std::string& filepath)
    {
        std::ifstream file(filepath);
        int count = 0;
        std::string line;
        while (std::getline(file, line))
        {
            ++count;
        }
        return count;
    }
};

TEST_F(LoggerTest, SingletonInstance)
{
    // Test that instance() returns the same object
    Logger& logger1 = Logger::instance();
    Logger& logger2 = Logger::instance();

    EXPECT_EQ(&logger1, &logger2);
}

TEST_F(LoggerTest, DefaultLogLevel)
{
    // Default log level should be INFO
    EXPECT_EQ(Logger::instance().getLevel(), LogLevel::INFO);
}

TEST_F(LoggerTest, SetLogLevel)
{
    Logger::instance().setLevel(LogLevel::DEBUG);
    EXPECT_EQ(Logger::instance().getLevel(), LogLevel::DEBUG);

    Logger::instance().setLevel(LogLevel::ERROR);
    EXPECT_EQ(Logger::instance().getLevel(), LogLevel::ERROR);

    Logger::instance().setLevel(LogLevel::NONE);
    EXPECT_EQ(Logger::instance().getLevel(), LogLevel::NONE);
}

TEST_F(LoggerTest, ConsoleOutputControl)
{
    // Default should be enabled
    EXPECT_TRUE(Logger::instance().isConsoleOutputEnabled());

    Logger::instance().setConsoleOutput(false);
    EXPECT_FALSE(Logger::instance().isConsoleOutputEnabled());

    Logger::instance().setConsoleOutput(true);
    EXPECT_TRUE(Logger::instance().isConsoleOutputEnabled());
}

TEST_F(LoggerTest, LogToFile)
{
    // Set up logger to write to file
    EXPECT_TRUE(Logger::instance().setLogFile(testLogFile_));

    // Write some messages
    Logger::instance().info("Test info message");
    Logger::instance().warn("Test warning message");
    Logger::instance().error("Test error message");

    // Flush to ensure data is written
    Logger::instance().flush();

    // Read and verify file contents
    std::string contents = readLogFile(testLogFile_);
    EXPECT_FALSE(contents.empty());
    EXPECT_NE(contents.find("Test info message"), std::string::npos);
    EXPECT_NE(contents.find("Test warning message"), std::string::npos);
    EXPECT_NE(contents.find("Test error message"), std::string::npos);
}

TEST_F(LoggerTest, LogLevelFiltering)
{
    Logger::instance().setLogFile(testLogFile_);
    Logger::instance().setConsoleOutput(false);
    Logger::instance().setLevel(LogLevel::WARN);

    // Log messages at different levels
    Logger::instance().debug("Debug message");  // Should be filtered
    Logger::instance().info("Info message");    // Should be filtered
    Logger::instance().warn("Warn message");    // Should be logged
    Logger::instance().error("Error message");  // Should be logged

    Logger::instance().flush();

    std::string contents = readLogFile(testLogFile_);
    EXPECT_EQ(contents.find("Debug message"), std::string::npos);
    EXPECT_EQ(contents.find("Info message"), std::string::npos);
    EXPECT_NE(contents.find("Warn message"), std::string::npos);
    EXPECT_NE(contents.find("Error message"), std::string::npos);
}

TEST_F(LoggerTest, LogLevelNoneDisablesAll)
{
    Logger::instance().setLogFile(testLogFile_);
    Logger::instance().setConsoleOutput(false);
    Logger::instance().setLevel(LogLevel::NONE);

    // Try to log at all levels
    Logger::instance().debug("Debug");
    Logger::instance().info("Info");
    Logger::instance().warn("Warn");
    Logger::instance().error("Error");

    Logger::instance().flush();

    // File should be empty or not created
    int lineCount = countLinesInFile(testLogFile_);
    EXPECT_EQ(lineCount, 0);
}

TEST_F(LoggerTest, LogFormatContainsTimestamp)
{
    Logger::instance().setLogFile(testLogFile_);
    Logger::instance().setConsoleOutput(false);

    Logger::instance().info("Test message");
    Logger::instance().flush();

    std::string contents = readLogFile(testLogFile_);
    // Log should contain timestamp in format [YYYY-MM-DD HH:MM:SS]
    EXPECT_NE(contents.find("["), std::string::npos);
    EXPECT_NE(contents.find("]"), std::string::npos);
}

TEST_F(LoggerTest, LogFormatContainsLevel)
{
    Logger::instance().setLogFile(testLogFile_);
    Logger::instance().setConsoleOutput(false);
    Logger::instance().setLevel(
        LogLevel::DEBUG);  // Set to DEBUG to capture all levels

    Logger::instance().debug("Debug");
    Logger::instance().info("Info");
    Logger::instance().warn("Warn");
    Logger::instance().error("Error");
    Logger::instance().flush();

    std::string contents = readLogFile(testLogFile_);
    EXPECT_NE(contents.find("DEBUG"), std::string::npos);
    EXPECT_NE(contents.find("INFO"), std::string::npos);
    EXPECT_NE(contents.find("WARN"), std::string::npos);
    EXPECT_NE(contents.find("ERROR"), std::string::npos);
}

TEST_F(LoggerTest, ThreadSafety)
{
    Logger::instance().setLogFile(testLogFile_);
    Logger::instance().setConsoleOutput(false);
    Logger::instance().setLevel(LogLevel::DEBUG);

    const int numThreads = 10;
    const int messagesPerThread = 100;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(
            [i, messagesPerThread]()
            {
                for (int j = 0; j < messagesPerThread; ++j)
                {
                    Logger::instance().info("Thread " + std::to_string(i) +
                                            " message " + std::to_string(j));
                }
            });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    Logger::instance().flush();

    // Verify all messages were logged
    int lineCount = countLinesInFile(testLogFile_);
    EXPECT_EQ(lineCount, numThreads * messagesPerThread);
}

TEST_F(LoggerTest, InvalidLogFilePath)
{
    // Try to set invalid log file path
    bool result =
        Logger::instance().setLogFile("/invalid/path/that/does/not/exist.log");
    EXPECT_FALSE(result);
}

TEST_F(LoggerTest, CloseAndReopenLogFile)
{
    Logger::instance().setLogFile(testLogFile_);
    Logger::instance().info("First message");
    Logger::instance().flush();

    Logger::instance().close();

    // Reopen and write again
    Logger::instance().setLogFile(testLogFile_);
    Logger::instance().info("Second message");
    Logger::instance().flush();

    std::string contents = readLogFile(testLogFile_);
    EXPECT_NE(contents.find("Second message"), std::string::npos);
}
