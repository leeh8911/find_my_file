#ifndef CONTENT_SEARCHER_H
#define CONTENT_SEARCHER_H

#include <filesystem>
#include <string>

namespace fmf
{

/**
 * @brief Utility class for searching within file contents
 *
 * Provides static methods for content-based file searching.
 * Implements safety checks to avoid searching binary files and
 * limits search to reasonable file sizes.
 *
 * Features:
 * - Text file detection (heuristic-based)
 * - Size limit protection (MAX_SEARCH_SIZE = 10MB)
 * - Binary file skipping
 * - Case-sensitive/insensitive search
 * - Regular expression support
 *
 * Example usage:
 * @code
 *   bool found = ContentSearcher::searchInFile(
 *       "/path/file.cpp",
 *       "TODO",
 *       false,  // not regex
 *       false   // case-insensitive
 *   );
 * @endcode
 *
 * @note All methods are static and thread-safe
 * @note Automatically skips binary files
 * @note Returns false for files exceeding size limit
 *
 * @warning Large files are skipped to prevent memory issues
 */
class ContentSearcher
{
 public:
    /**
     * @brief Search for a pattern within a file's contents
     * @param filePath Path to the file to search
     * @param pattern Pattern to search for
     * @param useRegex Whether to use regex matching
     * @param caseSensitive Whether to perform case-sensitive search
     * @return true if pattern found in file, false otherwise
     */
    static bool searchInFile(const std::filesystem::path& filePath,
                             const std::string& pattern, bool useRegex = false,
                             bool caseSensitive = true);

    /**
     * @brief Check if a file is likely to be a text file
     * @param filePath Path to the file
     * @return true if file appears to be text, false otherwise
     */
    static bool isTextFile(const std::filesystem::path& filePath);

 private:
    /**
     * @brief Maximum file size to search (10 MB)
     */
    static constexpr std::uintmax_t MAX_SEARCH_SIZE = 10 * 1024 * 1024;

    /**
     * @brief Sample size for text file detection
     */
    static constexpr size_t SAMPLE_SIZE = 512;
};

}  // namespace fmf

#endif  // CONTENT_SEARCHER_H
