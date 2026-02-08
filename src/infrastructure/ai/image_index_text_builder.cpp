#include "infrastructure/ai/image_index_text_builder.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <unordered_set>

namespace fmf
{

IndexTextResult ImageIndexTextBuilder::build(
    const std::string& ocrText, const std::string& captionText) const
{
    std::string cleanedOcr = cleanText(ocrText);
    std::string cleanedCaption = cleanText(captionText);

    IndexTextResult result;
    result.ocrText = cleanedOcr;
    result.captionText = cleanedCaption;
    result.indexText =
        "[OCR]\n" + cleanedOcr + "\n[CAPTION]\n" + cleanedCaption;
    return result;
}

std::string ImageIndexTextBuilder::cleanText(const std::string& text) const
{
    std::istringstream stream(text);
    std::string line;
    std::unordered_set<std::string> seen;
    std::ostringstream out;
    bool first = true;

    while (std::getline(stream, line))
    {
        std::string cleaned = normalizeWhitespace(line);
        if (cleaned.empty() || isJunkLine(cleaned))
        {
            continue;
        }

        std::string key = cleaned;
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (seen.count(key) > 0)
        {
            continue;
        }
        seen.insert(key);

        if (!first)
        {
            out << "\n";
        }
        out << cleaned;
        first = false;
    }

    return out.str();
}

std::string ImageIndexTextBuilder::normalizeWhitespace(
    const std::string& text) const
{
    std::string output;
    bool inSpace = false;
    for (char c : text)
    {
        if (std::isspace(static_cast<unsigned char>(c)))
        {
            if (!inSpace)
            {
                output.push_back(' ');
                inSpace = true;
            }
        }
        else
        {
            output.push_back(c);
            inSpace = false;
        }
    }

    if (!output.empty() && output.back() == ' ')
    {
        output.pop_back();
    }

    if (!output.empty() && output.front() == ' ')
    {
        output.erase(output.begin());
    }

    return output;
}

bool ImageIndexTextBuilder::isJunkLine(const std::string& line) const
{
    static const std::regex punctuationOnly(R"(^[\W_]+$)");
    static const std::regex repeatedPunct(R"(([!?.\-_=])\1{2,})");

    if (line.size() <= 1)
    {
        for (char c : line)
        {
            if (std::isdigit(static_cast<unsigned char>(c)))
            {
                return false;
            }
        }
        return true;
    }

    if (std::regex_match(line, punctuationOnly))
    {
        return true;
    }

    return std::regex_match(line, repeatedPunct);
}

}  // namespace fmf
