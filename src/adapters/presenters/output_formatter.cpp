#include "adapters/presenters/output_formatter.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace fmf
{

/**
 * @brief Construct formatter with specified format and color settings
 */
OutputFormatter::OutputFormatter(OutputFormat format, bool useColor)
    : format_(format), useColor_(useColor)
{
}

/**
 * @brief Print search results in the configured format
 * @param results Search results to format
 * @param out Output stream (defaults to std::cout)
 */
void OutputFormatter::print(const SearchResult& results,
                            std::ostream& out) const
{
    switch (format_)
    {
        case OutputFormat::Default:
            printDefault(results, out);
            break;
        case OutputFormat::Detailed:
            printDetailed(results, out);
            break;
        case OutputFormat::JSON:
            printJSON(results, out);
            break;
    }
}

std::string OutputFormatter::colorCode(Color color) const
{
    if (!useColor_)
    {
        return "";
    }

    switch (color)
    {
        case Color::Reset:
            return "\033[0m";
        case Color::Red:
            return "\033[31m";
        case Color::Green:
            return "\033[32m";
        case Color::Blue:
            return "\033[34m";
        case Color::Yellow:
            return "\033[33m";
        case Color::Cyan:
            return "\033[36m";
        case Color::Magenta:
            return "\033[35m";
        case Color::Gray:
            return "\033[90m";
        default:
            return "";
    }
}

void OutputFormatter::printDefault(const SearchResult& results,
                                   std::ostream& out) const
{
    out << "\nFound " << results.size() << " items:\n";
    out << std::string(80, '-') << "\n";

    for (const auto& fileInfo : results)
    {
        Color color = getFileColor(fileInfo);
        std::string type = fileInfo.isDirectory()      ? "[DIR]"
                           : fileInfo.isSymbolicLink() ? "[LINK]"
                                                       : "[FILE]";

        out << colorCode(color) << std::left << std::setw(8) << type
            << colorCode(Color::Reset) << std::setw(12) << std::right
            << fileInfo.getSize() << "  " << fileInfo.getPath() << "\n";
    }

    out << std::string(80, '-') << "\n";
}

void OutputFormatter::printDetailed(const SearchResult& results,
                                    std::ostream& out) const
{
    out << "\nFound " << results.size() << " items:\n";
    out << std::string(80, '=') << "\n";

    for (const auto& fileInfo : results)
    {
        Color color = getFileColor(fileInfo);

        out << colorCode(color);
        out << "Path: " << fileInfo.getPath() << "\n";
        out << colorCode(Color::Reset);

        out << "  Type: ";
        if (fileInfo.isDirectory())
        {
            out << "Directory\n";
        }
        else if (fileInfo.isSymbolicLink())
        {
            out << "Symbolic Link\n";
        }
        else
        {
            out << "Regular File\n";
        }

        out << "  Size: " << formatSize(fileInfo.getSize()) << " ("
            << fileInfo.getSize() << " bytes)\n";
        out << "  Modified: " << formatTime(fileInfo.getLastWriteTime())
            << "\n";
        out << std::string(80, '-') << "\n";
    }

    out << std::string(80, '=') << "\n";
}

void OutputFormatter::printJSON(const SearchResult& results,
                                std::ostream& out) const
{
    out << "{\n";
    out << "  \"count\": " << results.size() << ",\n";
    out << "  \"files\": [\n";

    bool first = true;
    for (const auto& fileInfo : results)
    {
        if (!first)
        {
            out << ",\n";
        }
        first = false;

        out << "    {\n";
        out << "      \"path\": \"" << fileInfo.getPath() << "\",\n";
        out << "      \"name\": \"" << fileInfo.getFileName() << "\",\n";

        std::string type = fileInfo.isDirectory()      ? "directory"
                           : fileInfo.isSymbolicLink() ? "symlink"
                                                       : "file";
        out << "      \"type\": \"" << type << "\",\n";
        out << "      \"size\": " << fileInfo.getSize() << ",\n";

        // Convert file time to Unix timestamp
        auto sctp =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                fileInfo.getLastWriteTime() -
                std::filesystem::file_time_type::clock::now() +
                std::chrono::system_clock::now());
        auto timestamp = std::chrono::system_clock::to_time_t(sctp);
        out << "      \"modified\": " << timestamp << "\n";
        out << "    }";
    }

    out << "\n  ]\n";
    out << "}\n";
}

std::string OutputFormatter::formatSize(size_t bytes) const
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 4)
    {
        size /= 1024.0;
        unitIndex++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " "
        << units[unitIndex];
    return oss.str();
}

std::string OutputFormatter::formatTime(
    const std::filesystem::file_time_type& time) const
{
    // Convert file_time to system_clock time
    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            time - std::filesystem::file_time_type::clock::now() +
            std::chrono::system_clock::now());
    auto tt = std::chrono::system_clock::to_time_t(sctp);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

Color OutputFormatter::getFileColor(const FileInfo& fileInfo) const
{
    if (fileInfo.isDirectory())
    {
        return Color::Blue;
    }
    else if (fileInfo.isSymbolicLink())
    {
        return Color::Cyan;
    }
    else if (fileInfo.getFileName().find(".cpp") != std::string::npos ||
             fileInfo.getFileName().find(".h") != std::string::npos)
    {
        return Color::Green;
    }
    else
    {
        return Color::Reset;
    }
}

}  // namespace fmf
