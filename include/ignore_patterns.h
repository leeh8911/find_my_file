#ifndef IGNORE_PATTERNS_H
#define IGNORE_PATTERNS_H

#include <filesystem>
#include <string>
#include <vector>

namespace fmf
{

/**
 * @brief Manages ignore patterns (.gitignore style)
 */
class IgnorePatterns
{
 public:
    /**
     * @brief Default constructor
     */
    IgnorePatterns() = default;

    /**
     * @brief Add a single ignore pattern
     * @param pattern Pattern to ignore (supports wildcards)
     */
    void addPattern(const std::string& pattern);

    /**
     * @brief Load patterns from a file
     * @param filePath Path to ignore file
     * @return true if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::filesystem::path& filePath);

    /**
     * @brief Check if a path should be ignored
     * @param path Path to check (can be relative or absolute)
     * @return true if should be ignored, false otherwise
     */
    bool shouldIgnore(const std::filesystem::path& path) const;

    /**
     * @brief Clear all patterns
     */
    void clear() { patterns_.clear(); }

    /**
     * @brief Check if any patterns are set
     */
    bool empty() const { return patterns_.empty(); }

    /**
     * @brief Get number of patterns
     */
    size_t size() const { return patterns_.size(); }

 private:
    std::vector<std::string> patterns_;

    /**
     * @brief Check if a single pattern matches a path
     * @param pattern Pattern to match
     * @param path Path to check
     * @return true if matches, false otherwise
     */
    bool matchesPattern(const std::string& pattern,
                        const std::filesystem::path& path) const;
};

}  // namespace fmf

#endif  // IGNORE_PATTERNS_H
