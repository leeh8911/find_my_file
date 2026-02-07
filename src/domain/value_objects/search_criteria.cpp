#include "domain/value_objects/search_criteria.h"

#include <algorithm>

namespace fmf
{

/**
 * @brief Set the name pattern for matching
 * @param pattern Wildcard or regex pattern
 * @note Pattern interpretation depends on useRegex flag
 */
void SearchCriteria::setNamePattern(const std::string& pattern)
{
    namePattern_ = pattern;
}

/**
 * @brief Set file extensions to filter by
 * @param extensions Vector of extensions (with or without leading dot)
 * @note Automatically adds leading dot if missing
 */
void SearchCriteria::setExtensions(const std::vector<std::string>& extensions)
{
    extensions_ = extensions;
    // Ensure all extensions start with a dot
    for (auto& ext : extensions_)
    {
        if (!ext.empty() && ext[0] != '.')
        {
            ext = "." + ext;
        }
    }
}

void SearchCriteria::addExtension(const std::string& extension)
{
    std::string ext = extension;
    if (!ext.empty() && ext[0] != '.')
    {
        ext = "." + ext;
    }
    extensions_.push_back(ext);
}

void SearchCriteria::setPathPattern(const std::string& pattern)
{
    pathPattern_ = pattern;
}

void SearchCriteria::setMinSize(std::uintmax_t size) { minSize_ = size; }

void SearchCriteria::setMaxSize(std::uintmax_t size) { maxSize_ = size; }

void SearchCriteria::setMinModifiedTime(
    const std::filesystem::file_time_type& time)
{
    minModifiedTime_ = time;
}

void SearchCriteria::setMaxModifiedTime(
    const std::filesystem::file_time_type& time)
{
    maxModifiedTime_ = time;
}

void SearchCriteria::setContentPattern(const std::string& pattern)
{
    contentPattern_ = pattern;
}

bool SearchCriteria::isEmpty() const
{
    return namePattern_.empty() && pathPattern_.empty() &&
           contentPattern_.empty() && extensions_.empty() &&
           !minSize_.has_value() && !maxSize_.has_value() &&
           !minModifiedTime_.has_value() && !maxModifiedTime_.has_value() &&
           !filesOnly_ && !directoriesOnly_;
}

}  // namespace fmf
