#include "pattern_matcher.h"

#include <algorithm>
#include <cctype>

namespace fmf
{

bool PatternMatcher::matchWildcard(const std::string& pattern,
                                   const std::string& text, bool caseSensitive)
{
    if (pattern.empty())
    {
        return text.empty();
    }

    std::string pat = caseSensitive ? pattern : toLower(pattern);
    std::string txt = caseSensitive ? text : toLower(text);

    return matchWildcardImpl(pat, txt, 0, 0);
}

bool PatternMatcher::matchExact(const std::string& pattern,
                                const std::string& text, bool caseSensitive)
{
    if (caseSensitive)
    {
        return pattern == text;
    }
    else
    {
        return toLower(pattern) == toLower(text);
    }
}

bool PatternMatcher::matchWildcardImpl(const std::string& pattern,
                                       const std::string& text, size_t pIdx,
                                       size_t tIdx)
{
    // Both strings consumed - success
    if (pIdx == pattern.length() && tIdx == text.length())
    {
        return true;
    }

    // Pattern consumed but text remains - fail unless we're in a * match
    if (pIdx == pattern.length())
    {
        return false;
    }

    // Handle * wildcard
    if (pattern[pIdx] == '*')
    {
        // Try matching * with empty string (skip the *)
        if (matchWildcardImpl(pattern, text, pIdx + 1, tIdx))
        {
            return true;
        }

        // Try matching * with one or more characters
        if (tIdx < text.length())
        {
            return matchWildcardImpl(pattern, text, pIdx, tIdx + 1);
        }

        return false;
    }

    // Text consumed but pattern remains
    if (tIdx == text.length())
    {
        // Only ok if remaining pattern is all *
        for (size_t i = pIdx; i < pattern.length(); ++i)
        {
            if (pattern[i] != '*')
            {
                return false;
            }
        }
        return true;
    }

    // Handle ? wildcard (matches exactly one character)
    if (pattern[pIdx] == '?')
    {
        return matchWildcardImpl(pattern, text, pIdx + 1, tIdx + 1);
    }

    // Handle regular character
    if (pattern[pIdx] == text[tIdx])
    {
        return matchWildcardImpl(pattern, text, pIdx + 1, tIdx + 1);
    }

    return false;
}

std::string PatternMatcher::toLower(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

}  // namespace fmf
