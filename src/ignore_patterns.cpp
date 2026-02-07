#include "ignore_patterns.h"

#include <fstream>

#include "pattern_matcher.h"

namespace fmf
{

void IgnorePatterns::addPattern(const std::string& pattern)
{
    // Skip empty lines and comments
    if (pattern.empty() || pattern[0] == '#')
    {
        return;
    }

    // Trim whitespace
    std::string trimmed = pattern;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

    if (!trimmed.empty())
    {
        patterns_.push_back(trimmed);
    }
}

bool IgnorePatterns::loadFromFile(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        addPattern(line);
    }

    return true;
}

bool IgnorePatterns::shouldIgnore(const std::filesystem::path& path) const
{
    if (patterns_.empty())
    {
        return false;
    }

    for (const auto& pattern : patterns_)
    {
        if (matchesPattern(pattern, path))
        {
            return true;
        }
    }

    return false;
}

bool IgnorePatterns::matchesPattern(const std::string& pattern,
                                   const std::filesystem::path& path) const
{
    std::string pathStr = path.string();
    std::string filename = path.filename().string();

    // If pattern ends with /, it only matches directories
    if (!pattern.empty() && pattern.back() == '/')
    {
        std::string dirPattern = pattern.substr(0, pattern.length() - 1);
        if (std::filesystem::is_directory(path))
        {
            if (PatternMatcher::matchWildcard(dirPattern, filename, true) ||
                PatternMatcher::matchWildcard(dirPattern, pathStr, true))
            {
                return true;
            }
        }
        return false;
    }

    // If pattern starts with /, match from root
    if (!pattern.empty() && pattern[0] == '/')
    {
        std::string rootPattern = pattern.substr(1);
        return PatternMatcher::matchWildcard(rootPattern, pathStr, true);
    }

    // Match against filename or anywhere in path
    if (PatternMatcher::matchWildcard(pattern, filename, true))
    {
        return true;
    }

    // Check if pattern matches any part of the path
    if (pathStr.find(pattern) != std::string::npos)
    {
        return true;
    }

    // Try wildcard match on full path
    if (PatternMatcher::matchWildcard(pattern, pathStr, true))
    {
        return true;
    }

    return false;
}

}  // namespace fmf
