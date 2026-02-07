#include "search_criteria.h"

#include <algorithm>

namespace fmf
{

void SearchCriteria::setNamePattern(const std::string& pattern)
{
    namePattern_ = pattern;
}

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

bool SearchCriteria::isEmpty() const
{
    return namePattern_.empty() && pathPattern_.empty() &&
           extensions_.empty() && !minSize_.has_value() &&
           !maxSize_.has_value() && !filesOnly_ && !directoriesOnly_;
}

}  // namespace fmf
