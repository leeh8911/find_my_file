/**
 * @file config_file.cpp
 * @brief Implementation of ConfigFile class
 */

#include "infrastructure/config/config_file.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace fmf
{

bool ConfigFile::load(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    if (parse(content))
    {
        loaded_ = true;
        loadedPath_ = filepath;
        return true;
    }

    return false;
}

bool ConfigFile::loadDefault()
{
    // Try current directory
    if (load(".findmyfilesrc"))
    {
        return true;
    }

    // Try home directory
    const char* home = std::getenv("HOME");
    if (home)
    {
        std::string homePath = std::string(home) + "/.findmyfilesrc";
        if (load(homePath))
        {
            return true;
        }

        // Try XDG config directory
        const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
        std::string configDir;
        if (xdgConfig)
        {
            configDir = std::string(xdgConfig) + "/find_my_files/config";
        }
        else
        {
            configDir = std::string(home) + "/.config/find_my_files/config";
        }

        if (load(configDir))
        {
            return true;
        }
    }

    return false;
}

bool ConfigFile::parse(const std::string& content)
{
    std::istringstream stream(content);
    std::string line;
    std::string currentSection;

    sections_.clear();

    while (std::getline(stream, line))
    {
        // Trim whitespace
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';')
        {
            continue;
        }

        // Section header [section_name]
        if (line[0] == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.length() - 2);
            currentSection = trim(currentSection);
            sections_[currentSection] = ConfigSection{currentSection, {}};
            continue;
        }

        // Key-value pair
        size_t pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));

            // Remove inline comments
            size_t commentPos = value.find('#');
            if (commentPos != std::string::npos)
            {
                value = trim(value.substr(0, commentPos));
            }
            commentPos = value.find(';');
            if (commentPos != std::string::npos)
            {
                value = trim(value.substr(0, commentPos));
            }

            if (!currentSection.empty())
            {
                sections_[currentSection].values[key] = value;
            }
        }
    }

    return true;
}

std::optional<ApplicationConfig> ConfigFile::getDefaultConfig() const
{
    if (!loaded_)
    {
        return std::nullopt;
    }

    auto it = sections_.find("default");
    if (it == sections_.end())
    {
        return std::nullopt;
    }

    return sectionToConfig(it->second);
}

std::optional<ApplicationConfig> ConfigFile::getSavedSearch(
    const std::string& name) const
{
    if (!loaded_)
    {
        return std::nullopt;
    }

    // Look for section named "search.{name}"
    std::string sectionName = "search." + name;
    auto it = sections_.find(sectionName);
    if (it == sections_.end())
    {
        return std::nullopt;
    }

    return sectionToConfig(it->second);
}

std::vector<std::string> ConfigFile::getSavedSearchNames() const
{
    std::vector<std::string> names;

    for (const auto& [sectionName, section] : sections_)
    {
        if (sectionName.rfind("search.", 0) == 0)  // starts with "search."
        {
            names.push_back(sectionName.substr(7));  // Remove "search." prefix
        }
    }

    return names;
}

std::optional<ApplicationConfig> ConfigFile::sectionToConfig(
    const ConfigSection& section) const
{
    ApplicationConfig config;

    for (const auto& [key, value] : section.values)
    {
        if (key == "recursive")
        {
            config.recursive = parseBool(value);
        }
        else if (key == "follow_links")
        {
            config.followLinks = parseBool(value);
        }
        else if (key == "max_depth")
        {
            config.maxDepth = std::stoi(value);
        }
        else if (key == "threads")
        {
            config.threadCount = std::stoul(value);
        }
        else if (key == "ignore_file")
        {
            config.ignoreFile = value;
        }
        else if (key == "format")
        {
            if (value == "default")
            {
                config.outputFormat = OutputFormat::Default;
            }
            else if (value == "detailed")
            {
                config.outputFormat = OutputFormat::Detailed;
            }
            else if (value == "json")
            {
                config.outputFormat = OutputFormat::JSON;
            }
        }
        else if (key == "color")
        {
            config.useColor = parseBool(value);
        }
        else if (key == "verbosity")
        {
            config.verbosity = std::stoi(value);
        }
        else if (key == "log_file")
        {
            config.logFile = value;
        }
        else if (key == "semantic_search")
        {
            config.semanticSearchQuery = value;
        }
        else if (key == "top_k")
        {
            config.semanticTopK = std::stoul(value);
        }
        // Search criteria
        else if (key == "name")
        {
            config.criteria.setNamePattern(value);
        }
        else if (key == "extensions")
        {
            // Parse comma-separated extensions
            std::istringstream iss(value);
            std::string ext;
            while (std::getline(iss, ext, ','))
            {
                ext = trim(ext);
                if (!ext.empty())
                {
                    config.criteria.addExtension(ext);
                }
            }
        }
        else if (key == "path")
        {
            config.criteria.setPathPattern(value);
        }
        else if (key == "ignore_case")
        {
            config.criteria.setCaseSensitive(!parseBool(value));
        }
        else if (key == "use_regex")
        {
            config.criteria.setUseRegex(parseBool(value));
        }
        else if (key == "content")
        {
            config.criteria.setContentPattern(value);
        }
        else if (key == "files_only")
        {
            config.criteria.setFilesOnly(parseBool(value));
        }
        else if (key == "directories_only")
        {
            config.criteria.setDirectoriesOnly(parseBool(value));
        }
        else if (key == "min_size")
        {
            config.criteria.setMinSize(std::stoull(value));
        }
        else if (key == "max_size")
        {
            config.criteria.setMaxSize(std::stoull(value));
        }
    }

    return config;
}

bool ConfigFile::parseBool(const std::string& value) const
{
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return lower == "true" || lower == "yes" || lower == "1" || lower == "on";
}

std::string ConfigFile::trim(const std::string& str) const
{
    const char* whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
    {
        return "";
    }
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

}  // namespace fmf
