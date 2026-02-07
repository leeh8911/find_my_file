#include <iomanip>
#include <iostream>

#include "file_info.h"
#include "file_scanner.h"
#include "ignore_patterns.h"
#include "output_formatter.h"
#include "search_criteria.h"
#include "search_result.h"

using namespace fmf;

void printUsage(const char* programName)
{
    std::cout
        << "Usage: " << programName << " [OPTIONS] <directory>\n"
        << "\nSearch Options:\n"
        << "  -n, --name PATTERN   Search by file name (supports wildcards * "
           "and ?)\n"
        << "  -e, --ext EXT        Filter by extension (e.g., .cpp, .h)\n"
        << "  -p, --path PATTERN   Search by path pattern\n"
        << "  -i, --ignore-case    Case-insensitive search\n"
        << "  -x, --regex          Use regular expressions for pattern "
           "matching\n"
        << "  -c, --content TEXT   Search within file contents\n"
        << "\nFile Type Options:\n"
        << "  -f, --files-only     Show only files (no directories)\n"
        << "  -D, --dirs-only      Show only directories\n"
        << "\nSize Options:\n"
        << "  --min-size SIZE      Minimum file size in bytes\n"
        << "  --max-size SIZE      Maximum file size in bytes\n"
        << "\nTraversal Options:\n"
        << "  -r, --recursive      Scan directories recursively\n"
        << "  -d, --max-depth N    Maximum recursion depth (use with -r)\n"
        << "  -l, --follow-links   Follow symbolic links\n"
        << "  --ignore FILE        Use ignore patterns from file (.gitignore "
           "style)\n"
        << "  -j, --threads N      Number of threads for parallel scanning "
           "(0=sequential)\n"
        << "\nOutput Options:\n"
        << "  --format FORMAT      Output format: default, detailed, json\n"
        << "  --color              Enable colored output\n"
        << "  --no-color           Disable colored output\n"
        << "\nOther Options:\n"
        << "  -h, --help           Display this help message\n"
        << "\nExamples:\n"
        << "  " << programName << " .\n"
        << "  " << programName << " -r /home/user\n"
        << "  " << programName << " -r -n '*.cpp' .\n"
        << "  " << programName << " -r -e .h -e .cpp src/\n"
        << "  " << programName << " -r -i -n '*test*' .\n"
        << "  " << programName << " -r --min-size 1000 --max-size 100000 .\n"
        << "  " << programName << " -r -x -n 'test_.*\\.cpp' .\n"
        << "  " << programName << " -r -c 'TODO' src/\n"
        << "  " << programName << " -r --ignore .gitignore .\n"
        << "  " << programName << " -r --format json . > results.json\n"
        << "  " << programName << " -r --format detailed --color .\n";
}

// Removed printResults() - now using OutputFormatter

// Removed printResults() - now using OutputFormatter

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    bool recursive = false;
    bool followLinks = false;
    int maxDepth = -1;
    size_t threadCount = 0;  // 0 = sequential
    std::string targetPath;
    std::string ignoreFile;
    OutputFormat outputFormat = OutputFormat::Default;
    bool useColor = false;

    SearchCriteria criteria;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-r" || arg == "--recursive")
        {
            recursive = true;
        }
        else if (arg == "-l" || arg == "--follow-links")
        {
            followLinks = true;
        }
        else if (arg == "-i" || arg == "--ignore-case")
        {
            criteria.setCaseSensitive(false);
        }
        else if (arg == "-f" || arg == "--files-only")
        {
            criteria.setFilesOnly(true);
        }
        else if (arg == "-D" || arg == "--dirs-only")
        {
            criteria.setDirectoriesOnly(true);
        }
        else if (arg == "-d" || arg == "--max-depth")
        {
            if (i + 1 < argc)
            {
                try
                {
                    maxDepth = std::stoi(argv[++i]);
                }
                catch (...)
                {
                    std::cerr << "Error: Invalid depth value\n";
                    return 1;
                }
            }
            else
            {
                std::cerr << "Error: --max-depth requires a numeric argument\n";
                return 1;
            }
        }
        else if (arg == "-n" || arg == "--name")
        {
            if (i + 1 < argc)
            {
                criteria.setNamePattern(argv[++i]);
            }
            else
            {
                std::cerr << "Error: --name requires a pattern argument\n";
                return 1;
            }
        }
        else if (arg == "-e" || arg == "--ext")
        {
            if (i + 1 < argc)
            {
                criteria.addExtension(argv[++i]);
            }
            else
            {
                std::cerr << "Error: --ext requires an extension argument\n";
                return 1;
            }
        }
        else if (arg == "-p" || arg == "--path")
        {
            if (i + 1 < argc)
            {
                criteria.setPathPattern(argv[++i]);
            }
            else
            {
                std::cerr << "Error: --path requires a pattern argument\n";
                return 1;
            }
        }
        else if (arg == "--min-size")
        {
            if (i + 1 < argc)
            {
                try
                {
                    criteria.setMinSize(std::stoull(argv[++i]));
                }
                catch (...)
                {
                    std::cerr << "Error: Invalid min-size value\n";
                    return 1;
                }
            }
            else
            {
                std::cerr << "Error: --min-size requires a numeric argument\n";
                return 1;
            }
        }
        else if (arg == "--max-size")
        {
            if (i + 1 < argc)
            {
                try
                {
                    criteria.setMaxSize(std::stoull(argv[++i]));
                }
                catch (...)
                {
                    std::cerr << "Error: Invalid max-size value\n";
                    return 1;
                }
            }
            else
            {
                std::cerr << "Error: --max-size requires a numeric argument\n";
                return 1;
            }
        }
        else if (arg == "-x" || arg == "--regex")
        {
            criteria.setUseRegex(true);
        }
        else if (arg == "-c" || arg == "--content")
        {
            if (i + 1 < argc)
            {
                criteria.setContentPattern(argv[++i]);
            }
            else
            {
                std::cerr << "Error: --content requires a pattern argument\n";
                return 1;
            }
        }
        else if (arg == "--ignore")
        {
            if (i + 1 < argc)
            {
                ignoreFile = argv[++i];
            }
            else
            {
                std::cerr << "Error: --ignore requires a file path\n";
                return 1;
            }
        }
        else if (arg == "-j" || arg == "--threads")
        {
            if (i + 1 < argc)
            {
                try
                {
                    int threads = std::stoi(argv[++i]);
                    if (threads < 0)
                    {
                        std::cerr
                            << "Error: Thread count must be non-negative\n";
                        return 1;
                    }
                    threadCount = static_cast<size_t>(threads);
                }
                catch (...)
                {
                    std::cerr << "Error: Invalid thread count value\n";
                    return 1;
                }
            }
            else
            {
                std::cerr << "Error: --threads requires a numeric argument\n";
                return 1;
            }
        }
        else if (arg == "--format")
        {
            if (i + 1 < argc)
            {
                std::string format = argv[++i];
                if (format == "default")
                {
                    outputFormat = OutputFormat::Default;
                }
                else if (format == "detailed")
                {
                    outputFormat = OutputFormat::Detailed;
                }
                else if (format == "json")
                {
                    outputFormat = OutputFormat::JSON;
                }
                else
                {
                    std::cerr << "Error: Unknown format '" << format
                              << "'. Valid: default, detailed, json\n";
                    return 1;
                }
            }
            else
            {
                std::cerr << "Error: --format requires an argument\n";
                return 1;
            }
        }
        else if (arg == "--color")
        {
            useColor = true;
        }
        else if (arg == "--no-color")
        {
            useColor = false;
        }
        else if (arg[0] != '-')
        {
            targetPath = arg;
        }
        else
        {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (targetPath.empty())
    {
        std::cerr << "Error: No directory specified\n";
        printUsage(argv[0]);
        return 1;
    }

    // Create scanner and configure it
    FileScanner scanner;
    scanner.setFollowSymlinks(followLinks);

    // Load ignore patterns if specified
    if (!ignoreFile.empty())
    {
        IgnorePatterns ignorePatterns;
        if (ignorePatterns.loadFromFile(ignoreFile))
        {
            scanner.setIgnorePatterns(ignorePatterns);
            std::cout << "Loaded " << ignorePatterns.size()
                      << " ignore patterns from " << ignoreFile << "\n";
        }
        else
        {
            std::cerr << "Warning: Could not load ignore file: " << ignoreFile
                      << "\n";
        }
    }

    // Scan directory
    SearchResult results;
    try
    {
        std::cout << "Scanning";
        if (recursive)
        {
            std::cout << " recursively";
            if (maxDepth >= 0)
            {
                std::cout << " (max depth: " << maxDepth << ")";
            }
        }
        if (threadCount > 0 && recursive)
        {
            std::cout << " using " << threadCount << " threads";
        }
        std::cout << "...\n";

        // Use search if criteria is set, otherwise just scan
        if (!criteria.isEmpty())
        {
            results = scanner.search(targetPath, recursive, criteria, maxDepth,
                                     threadCount);
        }
        else
        {
            results = recursive
                          ? scanner.scanDirectoryRecursive(targetPath, maxDepth)
                          : scanner.scanDirectory(targetPath);
        }

        // Print results using OutputFormatter
        OutputFormatter formatter(outputFormat, useColor);
        formatter.print(results);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
