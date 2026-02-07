#include "file_scanner.h"

#include <iostream>

#include "content_searcher.h"
#include "pattern_matcher.h"

namespace fmf
{

SearchResult FileScanner::scanDirectory(const std::filesystem::path& dirPath)
{
    SearchResult result;

    if (!std::filesystem::exists(dirPath))
    {
        std::cerr << "Directory does not exist: " << dirPath << std::endl;
        return result;
    }

    if (!std::filesystem::is_directory(dirPath))
    {
        std::cerr << "Path is not a directory: " << dirPath << std::endl;
        return result;
    }

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath))
        {
            if (shouldProcess(entry.path()))
            {
                try
                {
                    FileInfo fileInfo(entry.path());
                    result.addFile(fileInfo);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error processing file " << entry.path()
                              << ": " << e.what() << std::endl;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
    }

    return result;
}

SearchResult FileScanner::scanDirectoryRecursive(
    const std::filesystem::path& dirPath, int maxDepth)
{
    SearchResult result;

    if (!std::filesystem::exists(dirPath))
    {
        std::cerr << "Directory does not exist: " << dirPath << std::endl;
        return result;
    }

    if (!std::filesystem::is_directory(dirPath))
    {
        std::cerr << "Path is not a directory: " << dirPath << std::endl;
        return result;
    }

    scanRecursiveImpl(dirPath, 0, maxDepth, result);
    return result;
}

SearchResult FileScanner::scanWithFilter(
    const std::filesystem::path& dirPath, bool recursive,
    std::function<bool(const FileInfo&)> filter)
{
    SearchResult allResults =
        recursive ? scanDirectoryRecursive(dirPath) : scanDirectory(dirPath);

    SearchResult filteredResults;
    for (const auto& fileInfo : allResults)
    {
        if (filter(fileInfo))
        {
            filteredResults.addFile(fileInfo);
        }
    }

    return filteredResults;
}

void FileScanner::scanRecursiveImpl(const std::filesystem::path& dirPath,
                                    int currentDepth, int maxDepth,
                                    SearchResult& result)
{
    // Check depth limit
    if (maxDepth >= 0 && currentDepth > maxDepth)
    {
        return;
    }

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath))
        {
            if (!shouldProcess(entry.path()))
            {
                continue;
            }

            try
            {
                FileInfo fileInfo(entry.path());
                result.addFile(fileInfo);

                // Recurse into directories
                if (fileInfo.isDirectory())
                {
                    scanRecursiveImpl(entry.path(), currentDepth + 1, maxDepth,
                                      result);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error processing file " << entry.path() << ": "
                          << e.what() << std::endl;
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Error scanning directory " << dirPath << ": " << e.what()
                  << std::endl;
    }
}

bool FileScanner::shouldProcess(const std::filesystem::path& path)
{
    // Skip symbolic links if configured
    if (!followSymlinks_ && std::filesystem::is_symlink(path))
    {
        return false;
    }

    // Check ignore patterns
    if (ignorePatterns_.shouldIgnore(path))
    {
        return false;
    }

    return true;
}

SearchResult FileScanner::search(const std::filesystem::path& dirPath,
                                 bool recursive, const SearchCriteria& criteria,
                                 int maxDepth)
{
    // Get all files first
    SearchResult allResults = recursive
                                  ? scanDirectoryRecursive(dirPath, maxDepth)
                                  : scanDirectory(dirPath);

    // If no criteria, return all
    if (criteria.isEmpty())
    {
        return allResults;
    }

    // Filter results by criteria
    SearchResult filteredResults;
    for (const auto& fileInfo : allResults)
    {
        if (matchesCriteria(fileInfo, criteria))
        {
            filteredResults.addFile(fileInfo);
        }
    }

    return filteredResults;
}

bool FileScanner::matchesCriteria(const FileInfo& fileInfo,
                                  const SearchCriteria& criteria)
{
    // Check file type filters
    if (criteria.isFilesOnly() && !fileInfo.isRegularFile())
    {
        return false;
    }
    if (criteria.isDirectoriesOnly() && !fileInfo.isDirectory())
    {
        return false;
    }

    // Check name pattern
    if (!criteria.getNamePattern().empty())
    {
        bool matches = false;

        if (criteria.isUseRegex())
        {
            // Use regex matching
            matches = PatternMatcher::matchRegex(criteria.getNamePattern(),
                                                 fileInfo.getFileName(),
                                                 criteria.isCaseSensitive());
        }
        else
        {
            // Use wildcard matching
            matches = PatternMatcher::matchWildcard(criteria.getNamePattern(),
                                                    fileInfo.getFileName(),
                                                    criteria.isCaseSensitive());
        }

        if (!matches)
        {
            return false;
        }
    }

    // Check path pattern
    if (!criteria.getPathPattern().empty())
    {
        bool matches = false;

        if (criteria.isUseRegex())
        {
            matches = PatternMatcher::matchRegex(criteria.getPathPattern(),
                                                 fileInfo.getPath(),
                                                 criteria.isCaseSensitive());
        }
        else
        {
            matches = PatternMatcher::matchWildcard(criteria.getPathPattern(),
                                                    fileInfo.getPath(),
                                                    criteria.isCaseSensitive());
        }

        if (!matches)
        {
            return false;
        }
    }

    // Check extensions
    if (criteria.hasExtensions())
    {
        bool extensionMatch = false;
        std::string fileExt = fileInfo.getExtension();

        for (const auto& ext : criteria.getExtensions())
        {
            if (criteria.isCaseSensitive())
            {
                if (fileExt == ext)
                {
                    extensionMatch = true;
                    break;
                }
            }
            else
            {
                if (PatternMatcher::matchExact(ext, fileExt, false))
                {
                    extensionMatch = true;
                    break;
                }
            }
        }

        if (!extensionMatch)
        {
            return false;
        }
    }

    // Check file size
    if (criteria.getMinSize().has_value())
    {
        if (fileInfo.getSize() < criteria.getMinSize().value())
        {
            return false;
        }
    }
    if (criteria.getMaxSize().has_value())
    {
        if (fileInfo.getSize() > criteria.getMaxSize().value())
        {
            return false;
        }
    }

    // Check modification time
    if (criteria.getMinModifiedTime().has_value())
    {
        if (fileInfo.getLastWriteTime() < criteria.getMinModifiedTime().value())
        {
            return false;
        }
    }
    if (criteria.getMaxModifiedTime().has_value())
    {
        if (fileInfo.getLastWriteTime() > criteria.getMaxModifiedTime().value())
        {
            return false;
        }
    }

    // Check content pattern (only for regular files)
    if (criteria.hasContentPattern() && fileInfo.isRegularFile())
    {
        if (!ContentSearcher::searchInFile(
                fileInfo.getPath(), criteria.getContentPattern(),
                criteria.isUseRegex(), criteria.isCaseSensitive()))
        {
            return false;
        }
    }

    return true;
}

}  // namespace fmf
