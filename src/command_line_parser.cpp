/**
 * @file command_line_parser.cpp
 * @brief Implementation of command line argument parser
 */

#include "command_line_parser.h"

#include <iostream>

namespace fmf
{

std::optional<ApplicationConfig> CommandLineParser::parse(int argc,
                                                          char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Error: No arguments provided\n\n";
        return std::nullopt;
    }

    ApplicationConfig config;

    // Parse all arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        // Check for help first
        if (arg == "-h" || arg == "--help")
        {
            return std::nullopt;  // Signal caller to print help
        }

        // Parse option (may increment i if consuming additional arguments)
        if (!parseOption(arg, argc, argv, i, config))
        {
            return std::nullopt;
        }
    }

    // Validate final configuration
    if (!validateConfig(config))
    {
        return std::nullopt;
    }

    return config;
}

bool CommandLineParser::parseOption(const std::string& arg, int argc,
                                    char* argv[], int& currentIndex,
                                    ApplicationConfig& config)
{
    // Recursive options
    if (arg == "-r" || arg == "--recursive")
    {
        config.recursive = true;
    }
    // Follow symbolic links
    else if (arg == "-l" || arg == "--follow-links")
    {
        config.followLinks = true;
    }
    // Case sensitivity
    else if (arg == "-i" || arg == "--ignore-case")
    {
        config.criteria.setCaseSensitive(false);
    }
    // File type filters
    else if (arg == "-f" || arg == "--files-only")
    {
        config.criteria.setFilesOnly(true);
    }
    else if (arg == "-D" || arg == "--dirs-only")
    {
        config.criteria.setDirectoriesOnly(true);
    }
    // Max depth
    else if (arg == "-d" || arg == "--max-depth")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --max-depth requires a value\n";
            return false;
        }
        try
        {
            config.maxDepth = std::stoi(argv[++currentIndex]);
        }
        catch (...)
        {
            std::cerr << "Error: Invalid depth value\n";
            return false;
        }
    }
    // Name pattern
    else if (arg == "-n" || arg == "--name")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --name requires a pattern argument\n";
            return false;
        }
        config.criteria.setNamePattern(argv[++currentIndex]);
    }
    // Extension filter
    else if (arg == "-e" || arg == "--ext")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --ext requires an extension argument\n";
            return false;
        }
        config.criteria.addExtension(argv[++currentIndex]);
    }
    // Path pattern
    else if (arg == "-p" || arg == "--path")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --path requires a pattern argument\n";
            return false;
        }
        config.criteria.setPathPattern(argv[++currentIndex]);
    }
    // Size filters
    else if (arg == "--min-size")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --min-size requires a value\n";
            return false;
        }
        try
        {
            config.criteria.setMinSize(std::stoull(argv[++currentIndex]));
        }
        catch (...)
        {
            std::cerr << "Error: Invalid size value\n";
            return false;
        }
    }
    else if (arg == "--max-size")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --max-size requires a value\n";
            return false;
        }
        try
        {
            config.criteria.setMaxSize(std::stoull(argv[++currentIndex]));
        }
        catch (...)
        {
            std::cerr << "Error: Invalid size value\n";
            return false;
        }
    }
    // Regex mode
    else if (arg == "-x" || arg == "--regex")
    {
        config.criteria.setUseRegex(true);
    }
    // Content search
    else if (arg == "-c" || arg == "--content")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --content requires a pattern argument\n";
            return false;
        }
        config.criteria.setContentPattern(argv[++currentIndex]);
    }
    // Ignore patterns file
    else if (arg == "--ignore")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --ignore requires a file path\n";
            return false;
        }
        config.ignoreFile = argv[++currentIndex];
    }
    // Thread count
    else if (arg == "-j" || arg == "--threads")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --threads requires a value\n";
            return false;
        }
        try
        {
            config.threadCount = std::stoull(argv[++currentIndex]);
        }
        catch (...)
        {
            std::cerr << "Error: Invalid thread count\n";
            return false;
        }
    }
    // Output format
    else if (arg == "--format")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --format requires an argument\n";
            return false;
        }
        std::string formatStr = argv[++currentIndex];
        auto format = parseFormatString(formatStr);
        if (!format)
        {
            std::cerr << "Error: Unknown format '" << formatStr
                      << "'. Valid: default, detailed, json\n";
            return false;
        }
        config.outputFormat = *format;
    }
    // Color options
    else if (arg == "--color")
    {
        config.useColor = true;
    }
    else if (arg == "--no-color")
    {
        config.useColor = false;
    }
    // Logging options
    else if (arg == "-v" || arg == "--verbose")
    {
        config.verbosity++;  // Allow multiple -v for increased verbosity
    }
    else if (arg == "--log-file")
    {
        if (currentIndex + 1 >= argc)
        {
            std::cerr << "Error: --log-file requires a path argument\n";
            return false;
        }
        config.logFile = argv[++currentIndex];
    }
    // Target path (no flag)
    else if (arg[0] != '-')
    {
        if (!config.targetPath.empty())
        {
            std::cerr << "Error: Multiple target paths specified\n";
            return false;
        }
        config.targetPath = arg;
    }
    // Unknown option
    else
    {
        std::cerr << "Error: Unknown option: " << arg << "\n";
        return false;
    }

    return true;
}

bool CommandLineParser::validateConfig(const ApplicationConfig& config) const
{
    if (config.targetPath.empty())
    {
        std::cerr << "Error: No directory specified\n";
        return false;
    }

    // Could add more validation here:
    // - Check if both filesOnly and directoriesOnly are set
    // - Validate path exists
    // - Check min/max size consistency
    // etc.

    if (config.criteria.isFilesOnly() && config.criteria.isDirectoriesOnly())
    {
        std::cerr << "Error: Cannot use both --files-only and --dirs-only\n";
        return false;
    }

    return true;
}

std::optional<OutputFormat> CommandLineParser::parseFormatString(
    const std::string& formatStr) const
{
    if (formatStr == "default")
    {
        return OutputFormat::Default;
    }
    else if (formatStr == "detailed")
    {
        return OutputFormat::Detailed;
    }
    else if (formatStr == "json")
    {
        return OutputFormat::JSON;
    }
    return std::nullopt;
}

void CommandLineParser::printUsage(const char* programName) const
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
        << "  -v, --verbose        Increase verbosity (-v for info, -vv for "
           "debug)\n"
        << "  --log-file FILE      Write logs to specified file\n"
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

}  // namespace fmf
