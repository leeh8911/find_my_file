#ifndef FMF_OUTPUT_FORMATTER_H
#define FMF_OUTPUT_FORMATTER_H

#pragma once

#include <iostream>
#include <string>

#include "domain/entities/search_result.h"

namespace fmf
{

/**
 * @brief Output format type
 */
enum class OutputFormat
{
    Default,   // Simple list with basic info
    Detailed,  // Detailed information (-l)
    JSON       // JSON format for scripting
};

/**
 * @brief Terminal color codes
 */
enum class Color
{
    Reset,
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,
    Gray
};

/**
 * @brief Output formatter for search results
 *
 * Handles different output formats and presentation styles.
 * Follows the Single Responsibility Principle (SRP) by only
 * handling output formatting, not search logic.
 *
 * Follows the Strategy Pattern for format selection:
 * - Default: Compact list with basic info
 * - Detailed: Full metadata per file
 * - JSON: Machine-readable structured output
 *
 * Features:
 * - Multiple output formats
 * - ANSI color support for terminals
 * - Human-readable size formatting
 * - ISO 8601 timestamp formatting
 *
 * Example usage:
 * @code
 *   SearchResult results = scanner.search(...);
 *   OutputFormatter formatter(OutputFormat::Detailed, true);
 *   formatter.print(results);  // Prints to stdout with colors
 *
 *   // Or to a file
 *   std::ofstream file("output.json");
 *   OutputFormatter jsonFormatter(OutputFormat::JSON, false);
 *   jsonFormatter.print(results, file);
 * @endcode
 *
 * @note Thread-safe for concurrent formatting of different results
 * @note Color codes automatically reset after each colored section
 */
class OutputFormatter
{
 public:
    /**
     * @brief Constructor
     * @param format Output format type
     * @param useColor Whether to use terminal colors
     */
    explicit OutputFormatter(OutputFormat format = OutputFormat::Default,
                             bool useColor = false);

    /**
     * @brief Format and print search results
     * @param results Search results to format
     * @param out Output stream
     */
    void print(const SearchResult& results,
               std::ostream& out = std::cout) const;

    /**
     * @brief Set output format
     * @param format New format
     */
    void setFormat(OutputFormat format) { format_ = format; }

    /**
     * @brief Get current output format
     * @return Current format
     */
    OutputFormat getFormat() const { return format_; }

    /**
     * @brief Enable/disable color output
     * @param useColor true to enable colors
     */
    void setUseColor(bool useColor) { useColor_ = useColor; }

    /**
     * @brief Check if color output is enabled
     * @return true if colors are enabled
     */
    bool getUseColor() const { return useColor_; }

    /**
     * @brief Get ANSI color code
     * @param color Color to get code for
     * @return ANSI escape sequence or empty string if colors disabled
     */
    std::string colorCode(Color color) const;

 private:
    OutputFormat format_;
    bool useColor_;

    /**
     * @brief Print in default format
     */
    void printDefault(const SearchResult& results, std::ostream& out) const;

    /**
     * @brief Print in detailed format
     */
    void printDetailed(const SearchResult& results, std::ostream& out) const;

    /**
     * @brief Print in JSON format
     */
    void printJSON(const SearchResult& results, std::ostream& out) const;

    /**
     * @brief Format file size for human-readable output
     * @param bytes Size in bytes
     * @return Formatted string (e.g., "1.5 KB")
     */
    std::string formatSize(size_t bytes) const;

    /**
     * @brief Format time for human-readable output
     * @param time File time
     * @return Formatted string (e.g., "2026-02-07 10:30:45")
     */
    std::string formatTime(const std::filesystem::file_time_type& time) const;

    /**
     * @brief Get color for file type
     * @param fileInfo File information
     * @return Appropriate color for the file type
     */
    Color getFileColor(const FileInfo& fileInfo) const;
};

}  // namespace fmf

#endif  // FMF_OUTPUT_FORMATTER_H
