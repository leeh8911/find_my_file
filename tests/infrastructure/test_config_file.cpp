/**
 * @file test_config_file.cpp
 * @brief Unit tests for ConfigFile class
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "infrastructure/config/config_file.h"

namespace fs = std::filesystem;

namespace fmf
{

/**
 * @brief Test fixture for ConfigFile tests
 */
class ConfigFileTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        // Create temporary test directory
        testDir_ = fs::temp_directory_path() / "fmf_test_config";
        fs::create_directories(testDir_);
    }

    void TearDown() override
    {
        // Clean up temporary files
        if (fs::exists(testDir_))
        {
            fs::remove_all(testDir_);
        }
    }

    /**
     * @brief Create a temporary config file
     * @param filename Filename for config
     * @param content Content of config file
     * @return Full path to created file
     */
    std::string createConfigFile(const std::string& filename,
                                  const std::string& content)
    {
        fs::path filepath = testDir_ / filename;
        std::ofstream file(filepath);
        file << content;
        file.close();
        return filepath.string();
    }

    fs::path testDir_;
};

/**
 * @brief Test: Load non-existent file
 */
TEST_F(ConfigFileTest, LoadNonExistentFile)
{
    ConfigFile config;

    EXPECT_FALSE(config.load("nonexistent.cfg"));
    EXPECT_FALSE(config.isLoaded());
    EXPECT_EQ(config.getLoadedPath(), "");
}

/**
 * @brief Test: Load empty config file
 */
TEST_F(ConfigFileTest, LoadEmptyFile)
{
    std::string filepath = createConfigFile("empty.cfg", "");

    ConfigFile config;
    EXPECT_TRUE(config.load(filepath));
    EXPECT_TRUE(config.isLoaded());
    EXPECT_EQ(config.getLoadedPath(), filepath);
}

/**
 * @brief Test: Parse default section
 */
TEST_F(ConfigFileTest, ParseDefaultSection)
{
    std::string content = R"(
[default]
recursive=true
follow_links=false
threads=8
format=detailed
color=true
verbosity=2
)";

    std::string filepath = createConfigFile("default.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto defaultConfig = config.getDefaultConfig();
    ASSERT_TRUE(defaultConfig.has_value());

    EXPECT_TRUE(defaultConfig->recursive);
    EXPECT_FALSE(defaultConfig->followLinks);
    EXPECT_EQ(defaultConfig->threadCount, 8);
    EXPECT_EQ(defaultConfig->outputFormat, OutputFormat::Detailed);
    EXPECT_TRUE(defaultConfig->useColor);
    EXPECT_EQ(defaultConfig->verbosity, 2);
}

/**
 * @brief Test: Parse search criteria
 */
TEST_F(ConfigFileTest, ParseSearchCriteria)
{
    std::string content = R"(
[search.cpp_files]
name=*.cpp
extensions=.cpp,.h,.hpp
use_regex=false
ignore_case=true
files_only=true
)";

    std::string filepath = createConfigFile("search.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto search = config.getSavedSearch("cpp_files");
    ASSERT_TRUE(search.has_value());

    EXPECT_EQ(search->criteria.getNamePattern(), "*.cpp");
    EXPECT_FALSE(search->criteria.isCaseSensitive());
    EXPECT_TRUE(search->criteria.isFilesOnly());
}

/**
 * @brief Test: Parse multiple sections
 */
TEST_F(ConfigFileTest, ParseMultipleSections)
{
    std::string content = R"(
[default]
recursive=true
threads=4

[search.todos]
content=TODO
recursive=true

[search.logs]
extensions=.log,.txt
directories_only=false
)";

    std::string filepath = createConfigFile("multi.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    // Check default section
    auto defaultConfig = config.getDefaultConfig();
    ASSERT_TRUE(defaultConfig.has_value());
    EXPECT_TRUE(defaultConfig->recursive);
    EXPECT_EQ(defaultConfig->threadCount, 4);

    // Check saved searches
    auto searchNames = config.getSavedSearchNames();
    EXPECT_EQ(searchNames.size(), 2);

    auto todos = config.getSavedSearch("todos");
    ASSERT_TRUE(todos.has_value());
    EXPECT_EQ(todos->criteria.getContentPattern(), "TODO");

    auto logs = config.getSavedSearch("logs");
    ASSERT_TRUE(logs.has_value());
    EXPECT_FALSE(logs->criteria.isDirectoriesOnly());
}

/**
 * @brief Test: Comments and whitespace handling
 */
TEST_F(ConfigFileTest, HandleCommentsAndWhitespace)
{
    std::string content = R"(
# This is a comment
[default]
  recursive = true   # inline comment
  threads = 4

; Another comment style

[search.test]
name=test.txt
)";

    std::string filepath = createConfigFile("comments.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto defaultConfig = config.getDefaultConfig();
    ASSERT_TRUE(defaultConfig.has_value());
    EXPECT_TRUE(defaultConfig->recursive);
    EXPECT_EQ(defaultConfig->threadCount, 4);
}

/**
 * @brief Test: Boolean parsing
 */
TEST_F(ConfigFileTest, ParseBooleanValues)
{
    std::string content = R"(
[default]
recursive=true
follow_links=false
use_regex=yes
color=no
files_only=1
directories_only=0
case_sensitive=on
)";

    std::string filepath = createConfigFile("bool.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto defaultConfig = config.getDefaultConfig();
    ASSERT_TRUE(defaultConfig.has_value());

    EXPECT_TRUE(defaultConfig->recursive);
    EXPECT_FALSE(defaultConfig->followLinks);
    EXPECT_TRUE(defaultConfig->useColor == false);  // "no"
}

/**
 * @brief Test: Output format parsing
 */
TEST_F(ConfigFileTest, ParseOutputFormat)
{
    std::string content1 = "[default]\nformat=default\n";
    std::string content2 = "[default]\nformat=detailed\n";
    std::string content3 = "[default]\nformat=json\n";

    std::string file1 = createConfigFile("fmt1.cfg", content1);
    std::string file2 = createConfigFile("fmt2.cfg", content2);
    std::string file3 = createConfigFile("fmt3.cfg", content3);

    ConfigFile config1;
    ASSERT_TRUE(config1.load(file1));
    auto cfg1 = config1.getDefaultConfig();
    ASSERT_TRUE(cfg1.has_value());
    EXPECT_EQ(cfg1->outputFormat, OutputFormat::Default);

    ConfigFile config2;
    ASSERT_TRUE(config2.load(file2));
    auto cfg2 = config2.getDefaultConfig();
    ASSERT_TRUE(cfg2.has_value());
    EXPECT_EQ(cfg2->outputFormat, OutputFormat::Detailed);

    ConfigFile config3;
    ASSERT_TRUE(config3.load(file3));
    auto cfg3 = config3.getDefaultConfig();
    ASSERT_TRUE(cfg3.has_value());
    EXPECT_EQ(cfg3->outputFormat, OutputFormat::JSON);
}

/**
 * @brief Test: Extension list parsing
 */
TEST_F(ConfigFileTest, ParseExtensionList)
{
    std::string content = R"(
[search.code]
extensions=.cpp,.h,.hpp,.c
)";

    std::string filepath = createConfigFile("ext.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto search = config.getSavedSearch("code");
    ASSERT_TRUE(search.has_value());
    // Extensions should be added to criteria
}

/**
 * @brief Test: File size parsing
 */
TEST_F(ConfigFileTest, ParseFileSizes)
{
    std::string content = R"(
[search.large]
min_size=1048576
max_size=10485760
)";

    std::string filepath = createConfigFile("size.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto search = config.getSavedSearch("large");
    ASSERT_TRUE(search.has_value());
    // Size filters should be applied
}

/**
 * @brief Test: Get saved search names
 */
TEST_F(ConfigFileTest, GetSavedSearchNames)
{
    std::string content = R"(
[default]
recursive=true

[search.cpp]
name=*.cpp

[search.python]
name=*.py

[search.logs]
extensions=.log
)";

    std::string filepath = createConfigFile("names.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto names = config.getSavedSearchNames();
    EXPECT_EQ(names.size(), 3);

    // Check that all names are present
    auto hasCpp = std::find(names.begin(), names.end(), "cpp") != names.end();
    auto hasPython =
        std::find(names.begin(), names.end(), "python") != names.end();
    auto hasLogs = std::find(names.begin(), names.end(), "logs") != names.end();

    EXPECT_TRUE(hasCpp);
    EXPECT_TRUE(hasPython);
    EXPECT_TRUE(hasLogs);
}

/**
 * @brief Test: Non-existent saved search
 */
TEST_F(ConfigFileTest, NonExistentSavedSearch)
{
    std::string content = R"(
[search.existing]
name=test.txt
)";

    std::string filepath = createConfigFile("single.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto search = config.getSavedSearch("nonexistent");
    EXPECT_FALSE(search.has_value());
}

/**
 * @brief Test: Max depth parsing
 */
TEST_F(ConfigFileTest, ParseMaxDepth)
{
    std::string content = R"(
[default]
max_depth=5
)";

    std::string filepath = createConfigFile("depth.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto defaultConfig = config.getDefaultConfig();
    ASSERT_TRUE(defaultConfig.has_value());
    EXPECT_EQ(defaultConfig->maxDepth, 5);
}

/**
 * @brief Test: Ignore file setting
 */
TEST_F(ConfigFileTest, ParseIgnoreFile)
{
    std::string content = R"(
[default]
ignore_file=.myignore
)";

    std::string filepath = createConfigFile("ignore.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto defaultConfig = config.getDefaultConfig();
    ASSERT_TRUE(defaultConfig.has_value());
    EXPECT_EQ(defaultConfig->ignoreFile, ".myignore");
}

/**
 * @brief Test: Log file setting
 */
TEST_F(ConfigFileTest, ParseLogFile)
{
    std::string content = R"(
[default]
log_file=/var/log/fmf.log
verbosity=3
)";

    std::string filepath = createConfigFile("log.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto defaultConfig = config.getDefaultConfig();
    ASSERT_TRUE(defaultConfig.has_value());
    EXPECT_EQ(defaultConfig->logFile, "/var/log/fmf.log");
    EXPECT_EQ(defaultConfig->verbosity, 3);
}

/**
 * @brief Test: Content search pattern
 */
TEST_F(ConfigFileTest, ParseContentPattern)
{
    std::string content = R"(
[search.fixme]
content=FIXME|TODO
use_regex=true
)";

    std::string filepath = createConfigFile("content.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto search = config.getSavedSearch("fixme");
    ASSERT_TRUE(search.has_value());
    EXPECT_EQ(search->criteria.getContentPattern(), "FIXME|TODO");
    EXPECT_TRUE(search->criteria.isUseRegex());
}

/**
 * @brief Test: Path pattern
 */
TEST_F(ConfigFileTest, ParsePathPattern)
{
    std::string content = R"(
[search.src]
path=src/.*
use_regex=true
)";

    std::string filepath = createConfigFile("path.cfg", content);

    ConfigFile config;
    ASSERT_TRUE(config.load(filepath));

    auto search = config.getSavedSearch("src");
    ASSERT_TRUE(search.has_value());
    EXPECT_EQ(search->criteria.getPathPattern(), "src/.*");
}

}  // namespace fmf
