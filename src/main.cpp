/**
 * @file main.cpp
 * @brief Main entry point for find_my_files application
 *
 * This file contains the main function which orchestrates the file searching
 * process by:
 * 1. Loading configuration from .findmyfilesrc (if exists)
 * 2. Parsing command line arguments using CommandLineParser
 * 3. Configuring and executing the file scanner
 * 4. Formatting and displaying results using OutputFormatter
 *
 * The main function follows the Dependency Inversion Principle by depending
 * on abstractions (SearchCriteria, OutputFormatter) rather than concrete
 * implementations.
 */

#include <iostream>

#include "command_line_parser.h"
#include "config_file.h"
#include "file_scanner.h"
#include "ignore_patterns.h"
#include "logger.h"
#include "output_formatter.h"

using namespace fmf;

/**
 * @brief Main entry point
 *
 * Orchestrates the entire file search workflow:
 * - Loads configuration from .findmyfilesrc (if exists)
 * - Parses CLI arguments using CommandLineParser (SRP)
 * - Merges config file and CLI settings (CLI overrides config file)
 * - Configures file scanner with search criteria
 * - Executes search with optional parallelization
 * - Formats and outputs results using OutputFormatter
 *
 * Configuration precedence: CLI args > config file > defaults
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, 1 on error
 */
int main(int argc, char* argv[])
{
    // Step 1: Try to load configuration from default locations
    ConfigFile configFile;
    ApplicationConfig defaultConfig;  // Start with empty default config

    if (configFile.loadDefault())
    {
        // Load default config from file if available
        auto fileConfig = configFile.getDefaultConfig();
        if (fileConfig.has_value())
        {
            defaultConfig = *fileConfig;
            std::cerr << "Loaded configuration from: "
                      << configFile.getLoadedPath() << "\n";
        }
    }

    // Step 2: Parse command line arguments
    // CLI arguments will override config file settings
    CommandLineParser parser;
    auto configOpt = parser.parse(argc, argv, defaultConfig);

    if (!configOpt)
    {
        // Parse failed or help requested
        parser.printUsage(argv[0]);
        return argc < 2 ? 1 : 0;  // Return 0 for help, 1 for error
    }

    const ApplicationConfig& config = *configOpt;

    // Configure Logger based on verbosity level
    auto& logger = Logger::instance();
    if (config.verbosity >= 2)
    {
        logger.setLevel(LogLevel::DEBUG);
    }
    else if (config.verbosity == 1)
    {
        logger.setLevel(LogLevel::INFO);
    }
    else
    {
        logger.setLevel(LogLevel::WARN);  // Default: only warnings and errors
    }

    // Set log file if specified
    if (!config.logFile.empty())
    {
        if (!logger.setLogFile(config.logFile))
        {
            std::cerr << "Warning: Could not open log file: " << config.logFile
                      << "\n";
        }
        else
        {
            logger.info("Logging to file: " + config.logFile);
        }
    }

    // Disable console output if not verbose (to avoid mixing with results)
    if (config.verbosity == 0)
    {
        logger.setConsoleOutput(false);
    }

    logger.info("Starting file search in: " + config.targetPath);
    logger.debug("Recursive: " + std::string(config.recursive ? "yes" : "no"));
    logger.debug("Follow links: " +
                 std::string(config.followLinks ? "yes" : "no"));
    logger.debug("Max depth: " + (config.maxDepth >= 0
                                      ? std::to_string(config.maxDepth)
                                      : "unlimited"));
    logger.debug("Thread count: " + std::to_string(config.threadCount));

    // Create and configure file scanner
    FileScanner scanner;
    scanner.setFollowSymlinks(config.followLinks);

    // Load ignore patterns if specified
    if (!config.ignoreFile.empty())
    {
        logger.debug("Loading ignore patterns from: " + config.ignoreFile);
        IgnorePatterns ignorePatterns;
        if (ignorePatterns.loadFromFile(config.ignoreFile))
        {
            scanner.setIgnorePatterns(ignorePatterns);
            logger.info("Loaded " + std::to_string(ignorePatterns.size()) +
                        " ignore patterns from " + config.ignoreFile);
            std::cout << "Loaded " << ignorePatterns.size()
                      << " ignore patterns from " << config.ignoreFile << "\n";
        }
        else
        {
            logger.warn("Could not load ignore file: " + config.ignoreFile);
            std::cerr << "Warning: Could not load ignore file: "
                      << config.ignoreFile << "\n";
        }
    }

    // Execute file search
    SearchResult results;
    try
    {
        logger.info("Starting file search operation");

        // Display scanning status
        std::cout << "Scanning";
        if (config.recursive)
        {
            std::cout << " recursively";
            if (config.maxDepth >= 0)
            {
                std::cout << " (max depth: " << config.maxDepth << ")";
            }
        }
        if (config.threadCount > 0)
        {
            std::cout << " with " << config.threadCount << " threads";
        }
        std::cout << "...\n";

        // Perform search based on whether criteria is set
        if (!config.criteria.isEmpty())
        {
            logger.debug("Performing search with criteria");
            results = scanner.search(config.targetPath, config.recursive,
                                     config.criteria, config.maxDepth,
                                     config.threadCount);
        }
        else
        {
            logger.debug("Performing simple directory scan");
            // Simple scan without filtering
            results = config.recursive
                          ? scanner.scanDirectoryRecursive(config.targetPath,
                                                           config.maxDepth)
                          : scanner.scanDirectory(config.targetPath);
        }

        logger.info("Search complete. Found " +
                    std::to_string(results.getFiles().size()) + " matches");

        // Format and print results using OutputFormatter (SRP: separated
        // formatting logic)
        OutputFormatter formatter(config.outputFormat, config.useColor);
        formatter.print(results);
    }
    catch (const std::exception& e)
    {
        logger.error("Exception occurred: " + std::string(e.what()));
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    logger.info("Application completed successfully");
    logger.close();
    return 0;
}
