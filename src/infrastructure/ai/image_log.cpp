#include "infrastructure/ai/image_log.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace fmf
{

namespace
{
std::string escapeJson(const std::string& value)
{
    std::ostringstream out;
    for (char c : value)
    {
        switch (c)
        {
            case '\\':
                out << "\\\\";
                break;
            case '"':
                out << "\\\"";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                out << c;
                break;
        }
    }
    return out.str();
}

std::string utcTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm tm = *std::gmtime(&now);
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}
}  // namespace

void logImageEvent(const std::string& event, const std::string& level,
                   const std::map<std::string, std::string>& fields)
{
    std::ostringstream out;
    out << "{\"ts\":\"" << utcTimestamp() << "\","
        << "\"level\":\"" << escapeJson(level) << "\","
        << "\"event\":\"" << escapeJson(event) << "\"";

    for (const auto& [key, value] : fields)
    {
        out << ",\"" << escapeJson(key) << "\":\"" << escapeJson(value)
            << "\"";
    }

    out << "}";
    std::cerr << out.str() << "\n";
}

}  // namespace fmf
