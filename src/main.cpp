/**
 * @file main.cpp
 * @brief Main entry point for find_my_files application
 *
 * This file contains the main function which orchestrates the file searching
 * process by:
 * 1. Parsing command line arguments using Command LineParser
 * 2. Configuring and executing the file scanner
 * 3. Formatting and displaying results using OutputFormatter
 *
 * The main function follows the Dependency Inversion Principle by depending
 * on abstractions (SearchCriteria, OutputFormatter) rather than concrete
 * implementations.
 */

#include <iostream>

#include "command_line_parser.h"
#include "file_scanner.h"
#include "ignore_patterns.h"
#include "output_formatter.h"

using namespace fmf;

/**
 * @brief Main entry point
 *
 * Orchestrates the entire file search workflow:
 * - Parses CLI arguments using CommandLineParser (SRP)
 * - Configures file scanner with search criteria
 * - Executes search with optional parallelization
 * - Formats and outputs results using OutputFormatter
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, 1 on error
 */
int main(int argc, char* argv[])
{
    // Parse command line arguments using CommandLineParser (SRP: separated
    // parsing logic)
    CommandLineParser parser;
    auto configOpt = parser.parse(argc, argv);

    if (!configOpt)
    {
        // Parse failed or help requested
        parser.printUsage(argv[0]);
        return argc < 2 ? 1 : 0;  // Return 0 for help, 1 for error
    }

    const ApplicationConfig& config = *configOpt;

    // Create and configure file scanner
    FileScanner scanner;
    scanner.setFollowSymlinks(config.followLinks);

    // Load ignore patterns if specified
    if (!config.ignoreFile.empty())
    {
        IgnorePatterns ignorePatterns;
        if (ignorePatterns.loadFromFile(config.ignoreFile))
        {
            scanner.setIgnorePatterns(ignorePatterns);
            std::cout << "Loaded " << ignorePatterns.size()
                      << " ignore patterns from " << config.ignoreFile << "\n";
        }
        else
        {
            std::cerr << "Warning: Could not load ignore file: "
                      << config.ignoreFile << "\n";
        }
    }

    // Execute file search
    SearchResult results;
    try
    {
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
            results = scanner.search(config.targetPath, config.recursive,
                                     config.criteria, config.maxDepth,
                                     config.threadCount);
        }
        else
        {
            // Simple scan without filtering
            results = config.recursive
                          ? scanner.scanDirectoryRecursive(config.targetPath,
                                                           config.maxDepth)
                          : scanner.scanDirectory(config.targetPath);
        }

        // Format and print results using OutputFormatter (SRP: separated
        // formatting logic)
        OutputFormatter formatter(config.outputFormat, config.useColor);
        formatter.print(results);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
