#ifndef FMF_IMAGE_INDEX_TEXT_BUILDER_H
#define FMF_IMAGE_INDEX_TEXT_BUILDER_H

#pragma once

#include <string>

namespace fmf
{

struct IndexTextResult
{
    std::string ocrText;
    std::string captionText;
    std::string indexText;
};

class ImageIndexTextBuilder
{
 public:
    static constexpr const char* MERGE_RULE_VERSION = "v1";

    IndexTextResult build(const std::string& ocrText,
                          const std::string& captionText) const;

 private:
    std::string cleanText(const std::string& text) const;
    std::string normalizeWhitespace(const std::string& text) const;
    bool isJunkLine(const std::string& line) const;
};

}  // namespace fmf

#endif  // FMF_IMAGE_INDEX_TEXT_BUILDER_H
