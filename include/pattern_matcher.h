#ifndef PATTERN_MATCHER_H
#define PATTERN_MATCHER_H

#include <regex>
#include <string>

namespace fmf
{

/**
 * @brief Utility class for pattern matching with wildcard support
 */
class PatternMatcher
{
 public:
    /**
     * @brief Match a string against a pattern with wildcards
     * @param pattern Pattern to match (* for any sequence, ? for single char)
     * @param text Text to match against the pattern
     * @param caseSensitive Whether to perform case-sensitive matching
     * @return true if the text matches the pattern, false otherwise
     */
    static bool matchWildcard(const std::string& pattern,
                              const std::string& text,
                              bool caseSensitive = true);

    /**
     * @brief Check if a string matches exactly
     * @param pattern Pattern to match
     * @param text Text to match
     * @param caseSensitive Whether to perform case-sensitive matching
     * @return true if exact match, false otherwise
     */
    static bool matchExact(const std::string& pattern, const std::string& text,
                           bool caseSensitive = true);

    /**
     * @brief Match a string against a regular expression
     * @param regexPattern Regular expression pattern
     * @param text Text to match against the pattern
     * @param caseSensitive Whether to perform case-sensitive matching
     * @return true if the text matches the regex, false otherwise
     * @throws std::regex_error if the pattern is invalid
     */
    static bool matchRegex(const std::string& regexPattern,
                          const std::string& text, bool caseSensitive = true);

 private:
    /**
     * @brief Internal recursive wildcard matching implementation
     * @param pattern Pattern with wildcards
     * @param text Text to match
     * @param pIdx Current position in pattern
     * @param tIdx Current position in text
     * @return true if match found, false otherwise
     */
    static bool matchWildcardImpl(const std::string& pattern,
                                  const std::string& text, size_t pIdx,
                                  size_t tIdx);

    /**
     * @brief Convert string to lowercase
     * @param str Input string
     * @return Lowercase version of the string
     */
    static std::string toLower(const std::string& str);
};

}  // namespace fmf

#endif  // PATTERN_MATCHER_H
