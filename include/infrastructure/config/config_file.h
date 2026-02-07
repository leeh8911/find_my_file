/**
 * @file config_file.h
 * @brief Configuration file parser for find_my_files
 *
 * Provides support for .findmyfilesrc configuration files.
 * Allows users to define default search parameters, saved searches,
 * and custom settings.
 *
 * Configuration format: INI-style
 * Example .findmyfilesrc:
 * @code
 * [default]
 * recursive=true
 * follow_links=false
 * threads=4
 * ignore_file=.gitignore
 * format=default
 * color=true
 *
 * [search.cpp_files]
 * name=*.cpp
 * extensions=.cpp,.h,.hpp
 * directories_only=false
 *
 * [search.todos]
 * content=TODO
 * recursive=true
 * @endcode
 *
 * SOLID Principles:
 * - SRP: Only handles configuration file parsing
 * - OCP: Extensible for new configuration options
 */

#ifndef FMF_CONFIG_FILE_H
#define FMF_CONFIG_FILE_H

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "adapters/cli/command_line_parser.h"

namespace fmf
{

/**
 * @brief Configuration section (e.g., [default], [search.name])
 */
struct ConfigSection
{
    std::string name;                           ///< Section name
    std::map<std::string, std::string> values;  ///< Key-value pairs
};

/**
 * @brief Configuration file parser
 *
 * Parses INI-style configuration files for find_my_files.
 * Provides default settings and saved search profiles.
 *
 * Example usage:
 * @code
 *   ConfigFile config;
 *   if (config.load(".findmyfilesrc")) {
 *       auto defaultConfig = config.getDefaultConfig();
 *       // Apply default config
 *   }
 *
 *   // Get saved search
 *   auto search = config.getSavedSearch("cpp_files");
 *   if (search) {
 *       // Use saved search parameters
 *   }
 * @endcode
 */
class ConfigFile
{
 public:
    /**
     * @brief Default constructor
     */
    ConfigFile() = default;

    /**
     * @brief Load configuration from file
     *
     * @param filepath Path to configuration file
     * @return true if loaded successfully, false otherwise
     */
    bool load(const std::string& filepath);

    /**
     * @brief Load configuration from default locations
     *
     * Searches for .findmyfilesrc in:
     * 1. Current directory
     * 2. Home directory (~/.findmyfilesrc)
     * 3. XDG config directory (~/.config/find_my_files/config)
     *
     * @return true if any config found and loaded
     */
    bool loadDefault();

    /**
     * @brief Get default configuration section
     *
     * @return ApplicationConfig with default settings from [default] section
     */
    std::optional<ApplicationConfig> getDefaultConfig() const;

    /**
     * @brief Get saved search configuration
     *
     * @param name Name of saved search (from [search.name] section)
     * @return ApplicationConfig with search settings
     */
    std::optional<ApplicationConfig> getSavedSearch(
        const std::string& name) const;

    /**
     * @brief Get all saved search names
     *
     * @return Vector of saved search names
     */
    std::vector<std::string> getSavedSearchNames() const;

    /**
     * @brief Check if configuration was loaded
     *
     * @return true if configuration file was loaded
     */
    bool isLoaded() const { return loaded_; }

    /**
     * @brief Get the path of loaded configuration file
     *
     * @return Path to loaded file, or empty string if not loaded
     */
    std::string getLoadedPath() const { return loadedPath_; }

 private:
    /**
     * @brief Parse configuration file content
     *
     * @param content File content
     * @return true if parsed successfully
     */
    bool parse(const std::string& content);

    /**
     * @brief Convert configuration section to ApplicationConfig
     *
     * @param section Configuration section
     * @return ApplicationConfig if valid
     */
    std::optional<ApplicationConfig> sectionToConfig(
        const ConfigSection& section) const;

    /**
     * @brief Parse boolean value from string
     *
     * @param value String value ("true", "false", "yes", "no", "1", "0")
     * @return bool value
     */
    bool parseBool(const std::string& value) const;

    /**
     * @brief Trim whitespace from string
     *
     * @param str String to trim
     * @return Trimmed string
     */
    std::string trim(const std::string& str) const;

    bool loaded_ = false;     ///< Whether config was loaded
    std::string loadedPath_;  ///< Path to loaded config file
    std::map<std::string, ConfigSection> sections_;  ///< Parsed sections
};

}  // namespace fmf

#endif  // FMF_CONFIG_FILE_H
