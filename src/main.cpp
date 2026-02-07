#include <iomanip>
#include <iostream>

#include "file_info.h"
#include "file_scanner.h"
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
        << "\nOther Options:\n"
        << "  -h, --help           Display this help message\n"
        << "\nExamples:\n"
        << "  " << programName << " .\n"
        << "  " << programName << " -r /home/user\n"
        << "  " << programName << " -r -n '*.cpp' .\n"
        << "  " << programName << " -r -e .h -e .cpp src/\n"
        << "  " << programName << " -r -i -n '*test*' .\n"
        << "  " << programName << " -r --min-size 1000 --max-size 100000 .\n";
}

void printResults(const SearchResult& results)
{
    std::cout << "\nFound " << results.size() << " items:\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& fileInfo : results)
    {
        std::string type = fileInfo.isDirectory()      ? "[DIR]"
                           : fileInfo.isSymbolicLink() ? "[LINK]"
                                                       : "[FILE]";

        std::cout << std::left << std::setw(8) << type << std::setw(12)
                  << std::right << fileInfo.getSize() << "  "
                  << fileInfo.getPath() << "\n";
    }

    std::cout << std::string(80, '-') << "\n";
}

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
    std::string targetPath;

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
        std::cout << "...\n";

        // Use search if criteria is set, otherwise just scan
        if (!criteria.isEmpty())
        {
            results = scanner.search(targetPath, recursive, criteria, maxDepth);
        }
        else
        {
            results = recursive
                          ? scanner.scanDirectoryRecursive(targetPath, maxDepth)
                          : scanner.scanDirectory(targetPath);
        }

        printResults(results);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
