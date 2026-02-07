#ifndef SEARCH_CRITERIA_H
#define SEARCH_CRITERIA_H

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace fmf
{

/**
 * @brief Search criteria for file filtering
 *
 * This class encapsulates all search parameters and filters.
 * Follows the Single Responsibility Principle (SRP) by only managing
 * search criteria without performing actual filtering logic.
 *
 * Uses the Builder pattern for fluent API configuration:
 * @code
 *   SearchCriteria criteria;
 *   criteria.setNamePattern("*.cpp")
 *          .setCaseSensitive(false)
 *          .setMinSize(1024);
 * @endcode
 *
 * @note Thread-safe for reading after configuration.
 * @note All setters return void for simplicity, but could be chained
 *       by returning *this in future refactoring.
 */
class SearchCriteria
{
 public:
    /**
     * @brief Default constructor
     */
    SearchCriteria() = default;

    /**
     * @brief Set the name pattern to search for
     * @param pattern Pattern to match (supports wildcards)
     */
    void setNamePattern(const std::string& pattern);

    /**
     * @brief Get the name pattern
     * @return Name pattern or empty if not set
     */
    std::string getNamePattern() const { return namePattern_; }

    /**
     * @brief Set case sensitivity for name matching
     * @param caseSensitive true for case-sensitive, false for case-insensitive
     */
    void setCaseSensitive(bool caseSensitive)
    {
        caseSensitive_ = caseSensitive;
    }

    /**
     * @brief Check if case sensitivity is enabled
     * @return true if case-sensitive, false otherwise
     */
    bool isCaseSensitive() const { return caseSensitive_; }

    /**
     * @brief Set file extensions to filter by
     * @param extensions Vector of extensions (e.g., {".cpp", ".h"})
     */
    void setExtensions(const std::vector<std::string>& extensions);

    /**
     * @brief Add a single extension to filter
     * @param extension Extension to add (e.g., ".cpp")
     */
    void addExtension(const std::string& extension);

    /**
     * @brief Get the list of extensions to filter by
     * @return Vector of extensions
     */
    const std::vector<std::string>& getExtensions() const
    {
        return extensions_;
    }

    /**
     * @brief Check if extension filtering is enabled
     * @return true if extensions are set, false otherwise
     */
    bool hasExtensions() const { return !extensions_.empty(); }

    /**
     * @brief Set path pattern to filter by
     * @param pattern Path pattern to match
     */
    void setPathPattern(const std::string& pattern);

    /**
     * @brief Get the path pattern
     * @return Path pattern or empty if not set
     */
    std::string getPathPattern() const { return pathPattern_; }

    /**
     * @brief Set minimum file size filter
     * @param size Minimum size in bytes
     */
    void setMinSize(std::uintmax_t size);

    /**
     * @brief Set maximum file size filter
     * @param size Maximum size in bytes
     */
    void setMaxSize(std::uintmax_t size);

    /**
     * @brief Get minimum file size
     * @return Minimum size or nullopt if not set
     */
    std::optional<std::uintmax_t> getMinSize() const { return minSize_; }

    /**
     * @brief Get maximum file size
     * @return Maximum size or nullopt if not set
     */
    std::optional<std::uintmax_t> getMaxSize() const { return maxSize_; }

    /**
     * @brief Set whether to include only files (exclude directories)
     */
    void setFilesOnly(bool filesOnly) { filesOnly_ = filesOnly; }

    /**
     * @brief Check if only files should be included
     */
    bool isFilesOnly() const { return filesOnly_; }

    /**
     * @brief Set whether to include only directories (exclude files)
     */
    void setDirectoriesOnly(bool directoriesOnly)
    {
        directoriesOnly_ = directoriesOnly;
    }

    /**
     * @brief Check if only directories should be included
     */
    bool isDirectoriesOnly() const { return directoriesOnly_; }

    /**
     * @brief Set whether to use regex for name pattern matching
     */
    void setUseRegex(bool useRegex) { useRegex_ = useRegex; }

    /**
     * @brief Check if regex matching is enabled
     */
    bool isUseRegex() const { return useRegex_; }

    /**
     * @brief Set minimum modification time filter
     * @param time Minimum last write time
     */
    void setMinModifiedTime(const std::filesystem::file_time_type& time);

    /**
     * @brief Set maximum modification time filter
     * @param time Maximum last write time
     */
    void setMaxModifiedTime(const std::filesystem::file_time_type& time);

    /**
     * @brief Get minimum modification time
     * @return Minimum time or nullopt if not set
     */
    std::optional<std::filesystem::file_time_type> getMinModifiedTime() const
    {
        return minModifiedTime_;
    }

    /**
     * @brief Get maximum modification time
     * @return Maximum time or nullopt if not set
     */
    std::optional<std::filesystem::file_time_type> getMaxModifiedTime() const
    {
        return maxModifiedTime_;
    }

    /**
     * @brief Set content pattern to search within files
     * @param pattern Pattern to search in file contents
     */
    void setContentPattern(const std::string& pattern);

    /**
     * @brief Get content pattern
     * @return Content pattern or empty if not set
     */
    std::string getContentPattern() const { return contentPattern_; }

    /**
     * @brief Check if content search is enabled
     */
    bool hasContentPattern() const { return !contentPattern_.empty(); }

    /**
     * @brief Check if any criteria is set
     * @return true if any search criteria is configured
     */
    bool isEmpty() const;

 private:
    std::string namePattern_;
    std::string pathPattern_;
    std::string contentPattern_;
    std::vector<std::string> extensions_;
    std::optional<std::uintmax_t> minSize_;
    std::optional<std::uintmax_t> maxSize_;
    std::optional<std::filesystem::file_time_type> minModifiedTime_;
    std::optional<std::filesystem::file_time_type> maxModifiedTime_;
    bool caseSensitive_ = true;
    bool filesOnly_ = false;
    bool directoriesOnly_ = false;
    bool useRegex_ = false;
};

}  // namespace fmf

#endif  // SEARCH_CRITERIA_H
