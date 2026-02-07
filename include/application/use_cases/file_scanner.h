#ifndef FILE_SCANNER_H
#define FILE_SCANNER_H

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

#include "domain/entities/file_info.h"
#include "domain/entities/search_result.h"
#include "domain/value_objects/search_criteria.h"
#include "application/services/ignore_patterns.h"
#include "infrastructure/threading/thread_pool.h"

namespace fmf
{

/**
 * @brief Scanner for traversing and searching files
 *
 * This class provides directory traversal and file searching capabilities.
 * It coordinates between PatternMatcher, ContentSearcher, and IgnorePatterns
 * to perform comprehensive file searches.
 *
 * SOLID Principles:
 * - SRP: Focuses on directory traversal and coordination
 * - OCP: Extensible through SearchCriteria without modifying scanner code
 * - DIP: Depends on SearchCriteria abstraction, not concrete implementations
 *
 * Features:
 * - Recursive directory traversal with depth control
 * - Parallel scanning using ThreadPool
 * - Symbolic link handling
 * - .gitignore-style ignore patterns
 * - Content-based search
 *
 * Example usage:
 * @code
 *   FileScanner scanner;
 *   scanner.setFollowSymlinks(true);
 *   SearchCriteria criteria;
 *   criteria.setNamePattern("*.cpp");
 *   auto results = scanner.search("/path", true, criteria);
 * @endcode
 *
 * @note Thread-safe when using parallel scanning
 * @note Configurable through setFollowSymlinks() and setIgnorePatterns()
 */
class FileScanner
{
 public:
    /**
     * @brief Default constructor
     */
    FileScanner() = default;

    /**
     * @brief Scan a directory non-recursively
     * @param dirPath Path to the directory
     * @return SearchResult containing found files
     */
    SearchResult scanDirectory(const std::filesystem::path& dirPath);

    /**
     * @brief Scan a directory recursively
     * @param dirPath Path to the directory
     * @param maxDepth Maximum depth to recurse (-1 for unlimited)
     * @return SearchResult containing found files
     */
    SearchResult scanDirectoryRecursive(const std::filesystem::path& dirPath,
                                        int maxDepth = -1);

    /**
     * @brief Scan with a custom filter predicate
     * @param dirPath Path to the directory
     * @param recursive Whether to scan recursively
     * @param filter Function that returns true for files to include
     * @return SearchResult containing filtered files
     */
    SearchResult scanWithFilter(const std::filesystem::path& dirPath,
                                bool recursive,
                                std::function<bool(const FileInfo&)> filter);

    /**
     * @brief Scan with search criteria
     * @param dirPath Path to the directory
     * @param recursive Whether to scan recursively
     * @param criteria Search criteria to apply
     * @param maxDepth Maximum depth for recursive scan (-1 for unlimited)
     * @param threadCount Number of threads for parallel scanning (0 for
     * sequential)
     * @return SearchResult containing files matching the criteria
     */
    SearchResult search(const std::filesystem::path& dirPath, bool recursive,
                        const SearchCriteria& criteria, int maxDepth = -1,
                        size_t threadCount = 0);

    /**
     * @brief Set whether to follow symbolic links
     * @param follow true to follow symlinks, false to skip them
     */
    void setFollowSymlinks(bool follow) { followSymlinks_ = follow; }

    /**
     * @brief Get whether symbolic links are followed
     * @return true if following symlinks, false otherwise
     */
    bool getFollowSymlinks() const { return followSymlinks_; }

    /**
     * @brief Set ignore patterns
     * @param patterns IgnorePatterns object
     */
    void setIgnorePatterns(const IgnorePatterns& patterns)
    {
        ignorePatterns_ = patterns;
    }

    /**
     * @brief Get ignore patterns
     */
    const IgnorePatterns& getIgnorePatterns() const { return ignorePatterns_; }

 private:
    bool followSymlinks_ = false;
    IgnorePatterns ignorePatterns_;

    /**
     * @brief Internal recursive scan implementation
     * @param dirPath Current directory path
     * @param currentDepth Current recursion depth
     * @param maxDepth Maximum depth (-1 for unlimited)
     * @param result SearchResult to accumulate files
     */
    void scanRecursiveImpl(const std::filesystem::path& dirPath,
                           int currentDepth, int maxDepth,
                           SearchResult& result);
    /**
     * @brief Parallel recursive scan implementation
     * @param dirPath Current directory path
     * @param currentDepth Current recursion depth
     * @param maxDepth Maximum depth (-1 for unlimited)
     * @param result Result container to populate
     * @param pool Thread pool for parallel execution
     */
    void scanRecursiveParallel(const std::filesystem::path& dirPath,
                               int currentDepth, int maxDepth,
                               SearchResult& result, ThreadPool& pool);
    /**
     * @brief Check if a path should be processed
     * @param path Path to check
     * @return true if should be processed, false to skip
     */
    bool shouldProcess(const std::filesystem::path& path);

    /**
     * @brief Check if a FileInfo matches the search criteria
     * @param fileInfo File to check
     * @param criteria Criteria to match against
     * @return true if matches, false otherwise
     */
    bool matchesCriteria(const FileInfo& fileInfo,
                         const SearchCriteria& criteria);
};

}  // namespace fmf

#endif  // FILE_SCANNER_H
