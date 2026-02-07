#include "content_searcher.h"

#include <fstream>
#include <regex>
#include <sstream>

#include "pattern_matcher.h"

namespace fmf
{

bool ContentSearcher::searchInFile(const std::filesystem::path& filePath,
                                  const std::string& pattern, bool useRegex,
                                  bool caseSensitive)
{
    // Check if file exists and is a regular file
    if (!std::filesystem::exists(filePath) ||
        !std::filesystem::is_regular_file(filePath))
    {
        return false;
    }

    // Check file size
    try
    {
        auto fileSize = std::filesystem::file_size(filePath);
        if (fileSize > MAX_SEARCH_SIZE || fileSize == 0)
        {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }

    // Check if it's likely a text file
    if (!isTextFile(filePath))
    {
        return false;
    }

    // Read and search file contents
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (useRegex)
        {
            if (PatternMatcher::matchRegex(pattern, line, caseSensitive))
            {
                return true;
            }
        }
        else
        {
            // Simple substring search
            std::string searchLine = line;
            std::string searchPattern = pattern;

            if (!caseSensitive)
            {
                std::transform(searchLine.begin(), searchLine.end(),
                             searchLine.begin(),
                             [](unsigned char c) { return std::tolower(c); });
                std::transform(searchPattern.begin(), searchPattern.end(),
                             searchPattern.begin(),
                             [](unsigned char c) { return std::tolower(c); });
            }

            if (searchLine.find(searchPattern) != std::string::npos)
            {
                return true;
            }
        }
    }

    return false;
}

bool ContentSearcher::isTextFile(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }

    // Read a sample of the file
    char buffer[SAMPLE_SIZE];
    file.read(buffer, SAMPLE_SIZE);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead == 0)
    {
        return false;
    }

    // Check for null bytes and non-printable characters
    int nullBytes = 0;
    int nonPrintable = 0;

    for (std::streamsize i = 0; i < bytesRead; ++i)
    {
        unsigned char c = static_cast<unsigned char>(buffer[i]);

        if (c == 0)
        {
            nullBytes++;
        }
        else if (c < 32 && c != '\n' && c != '\r' && c != '\t')
        {
            nonPrintable++;
        }
    }

    // If more than 10% null bytes or non-printable, likely binary
    double nullRatio = static_cast<double>(nullBytes) / bytesRead;
    double nonPrintableRatio = static_cast<double>(nonPrintable) / bytesRead;

    return nullRatio < 0.1 && nonPrintableRatio < 0.3;
}

}  // namespace fmf
