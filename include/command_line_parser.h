/**
 * @file command_line_parser.h
 * @brief Command line argument parser for find_my_files application
 *
 * This file defines the CommandLineParser class which is responsible for
 * parsing and validating command line arguments. This follows the Single
 * Responsibility Principle by separating CLI parsing from the main application
 * logic.
 */

#ifndef FMF_COMMAND_LINE_PARSER_H
#define FMF_COMMAND_LINE_PARSER_H

#pragma once

#include <string>
#include <vector>

#include "output_formatter.h"
#include "search_criteria.h"

namespace fmf
{

/**
 * @brief Configuration parsed from command line arguments
 *
 * Contains all settings and parameters extracted from CLI arguments.
 * This struct serves as a Data Transfer Object (DTO) to pass configuration
 * between the parser and the main application.
 */
struct ApplicationConfig
{
    bool recursive = false;    ///< Recursive directory scanning
    bool followLinks = false;  ///< Follow symbolic links
    int maxDepth = -1;         ///< Maximum recursion depth (-1 = unlimited)
    size_t threadCount = 0;    ///< Number of threads (0 = sequential)
    std::string targetPath;    ///< Target directory path
    std::string ignoreFile;    ///< Path to ignore patterns file
    OutputFormat outputFormat = OutputFormat::Default;  ///< Output format type
    bool useColor = false;    ///< Enable colored output
    SearchCriteria criteria;  ///< Search criteria and filters
};

/**
 * @brief Command line argument parser
 *
 * Parses command line arguments and produces an ApplicationConfig object.
 * This class encapsulates all CLI parsing logic, following the Single
 * Responsibility Principle.
 *
 * @note This class is designed to be stateless and can be reused for
 *       parsing multiple command lines if needed.
 *
 * Example usage:
 * @code
 *   CommandLineParser parser;
 *   auto config = parser.parse(argc, argv);
 *   if (!config) {
 *       std::cerr << "Invalid arguments\n";
 *       parser.printUsage(argv[0]);
 *       return 1;
 *   }
 * @endcode
 */
class CommandLineParser
{
 public:
    /**
     * @brief Default constructor
     */
    CommandLineParser() = default;

    /**
     * @brief Parse command line arguments
     *
     * Parses the given command line arguments and constructs an
     * ApplicationConfig object containing all the parsed settings.
     *
     * @param argc Argument count from main()
     * @param argv Argument vector from main()
     * @return ApplicationConfig if successful, std::nullopt on error
     *
     * @note On parse error, an error message is printed to std::cerr
     * @note Returns std::nullopt for help request (-h/--help)
     */
    std::optional<ApplicationConfig> parse(int argc, char* argv[]);

    /**
     * @brief Print usage information
     *
     * Displays comprehensive help text showing all available options,
     * their descriptions, and usage examples.
     *
     * @param programName Name of the program (usually argv[0])
     */
    void printUsage(const char* programName) const;

 private:
    /**
     * @brief Parse a single command line option
     *
     * Processes one argument and updates the config accordingly.
     * May consume additional arguments from argv if the option requires values.
     *
     * @param arg Current argument being processed
     * @param argc Total argument count
     * @param argv Argument vector
     * @param currentIndex Current position in argv (may be incremented)
     * @param config Configuration object to update
     * @return true if parsing successful, false on error
     */
    bool parseOption(const std::string& arg, int argc, char* argv[],
                     int& currentIndex, ApplicationConfig& config);

    /**
     * @brief Validate the parsed configuration
     *
     * Ensures the configuration is valid and complete.
     * For example, checks that a target path was provided.
     *
     * @param config Configuration to validate
     * @return true if valid, false otherwise
     */
    bool validateConfig(const ApplicationConfig& config) const;

    /**
     * @brief Parse format string to OutputFormat enum
     *
     * @param formatStr Format string ("default", "detailed", or "json")
     * @return OutputFormat enum value, or std::nullopt if invalid
     */
    std::optional<OutputFormat> parseFormatString(
        const std::string& formatStr) const;
};

}  // namespace fmf

#endif  // FMF_COMMAND_LINE_PARSER_H
