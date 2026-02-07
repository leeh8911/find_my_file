#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "output_formatter.h"

using namespace fmf;

class OutputFormatterTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        // Create test files
        testDir_ =
            std::filesystem::temp_directory_path() / "fmf_formatter_test";
        std::filesystem::create_directories(testDir_);

        testFile1_ = testDir_ / "file1.txt";
        testFile2_ = testDir_ / "file2.cpp";
        testFile3_ = testDir_ / "file3.h";

        std::ofstream(testFile1_) << "test content 1";
        std::ofstream(testFile2_) << "test content 2";
        std::ofstream(testFile3_) << "test content 3";

        // Create test results
        results_.addFile(FileInfo(testFile1_));
        results_.addFile(FileInfo(testFile2_));
        results_.addFile(FileInfo(testDir_));
    }

    void TearDown() override { std::filesystem::remove_all(testDir_); }

    std::filesystem::path testDir_;
    std::filesystem::path testFile1_;
    std::filesystem::path testFile2_;
    std::filesystem::path testFile3_;
    SearchResult results_;
};

TEST_F(OutputFormatterTest, DefaultConstructor)
{
    OutputFormatter formatter;
    EXPECT_EQ(formatter.getFormat(), OutputFormat::Default);
    EXPECT_FALSE(formatter.getUseColor());
}

TEST_F(OutputFormatterTest, ConstructorWithParameters)
{
    OutputFormatter formatter(OutputFormat::Detailed, true);
    EXPECT_EQ(formatter.getFormat(), OutputFormat::Detailed);
    EXPECT_TRUE(formatter.getUseColor());
}

TEST_F(OutputFormatterTest, SetFormat)
{
    OutputFormatter formatter;
    formatter.setFormat(OutputFormat::JSON);
    EXPECT_EQ(formatter.getFormat(), OutputFormat::JSON);
}

TEST_F(OutputFormatterTest, SetUseColor)
{
    OutputFormatter formatter;
    formatter.setUseColor(true);
    EXPECT_TRUE(formatter.getUseColor());

    formatter.setUseColor(false);
    EXPECT_FALSE(formatter.getUseColor());
}

TEST_F(OutputFormatterTest, ColorCodeWithoutColors)
{
    OutputFormatter formatter(OutputFormat::Default, false);
    EXPECT_EQ(formatter.colorCode(Color::Red), "");
    EXPECT_EQ(formatter.colorCode(Color::Green), "");
    EXPECT_EQ(formatter.colorCode(Color::Blue), "");
}

TEST_F(OutputFormatterTest, ColorCodeWithColors)
{
    OutputFormatter formatter(OutputFormat::Default, true);
    EXPECT_EQ(formatter.colorCode(Color::Reset), "\033[0m");
    EXPECT_EQ(formatter.colorCode(Color::Red), "\033[31m");
    EXPECT_EQ(formatter.colorCode(Color::Green), "\033[32m");
    EXPECT_EQ(formatter.colorCode(Color::Blue), "\033[34m");
    EXPECT_EQ(formatter.colorCode(Color::Yellow), "\033[33m");
    EXPECT_EQ(formatter.colorCode(Color::Cyan), "\033[36m");
    EXPECT_EQ(formatter.colorCode(Color::Magenta), "\033[35m");
    EXPECT_EQ(formatter.colorCode(Color::Gray), "\033[90m");
}

TEST_F(OutputFormatterTest, PrintDefaultFormat)
{
    OutputFormatter formatter(OutputFormat::Default, false);
    std::ostringstream oss;
    formatter.print(results_, oss);

    std::string output = oss.str();
    EXPECT_NE(output.find("Found 3 items"), std::string::npos);
    EXPECT_NE(output.find("file1.txt"), std::string::npos);
    EXPECT_NE(output.find("file2.cpp"), std::string::npos);
    EXPECT_NE(output.find("[FILE]"), std::string::npos);
    EXPECT_NE(output.find("[DIR]"), std::string::npos);
}

TEST_F(OutputFormatterTest, PrintDefaultFormatWithColors)
{
    OutputFormatter formatter(OutputFormat::Default, true);
    std::ostringstream oss;
    formatter.print(results_, oss);

    std::string output = oss.str();
    EXPECT_NE(output.find("\033["), std::string::npos);  // Contains color codes
    EXPECT_NE(output.find("file1.txt"), std::string::npos);
}

TEST_F(OutputFormatterTest, PrintDetailedFormat)
{
    OutputFormatter formatter(OutputFormat::Detailed, false);
    std::ostringstream oss;
    formatter.print(results_, oss);

    std::string output = oss.str();
    EXPECT_NE(output.find("Found 3 items"), std::string::npos);
    EXPECT_NE(output.find("Path:"), std::string::npos);
    EXPECT_NE(output.find("Type:"), std::string::npos);
    EXPECT_NE(output.find("Size:"), std::string::npos);
    EXPECT_NE(output.find("Modified:"), std::string::npos);
    EXPECT_NE(output.find("file1.txt"), std::string::npos);
    EXPECT_NE(output.find("Regular File"), std::string::npos);
    EXPECT_NE(output.find("Directory"), std::string::npos);
}

TEST_F(OutputFormatterTest, PrintJSONFormat)
{
    OutputFormatter formatter(OutputFormat::JSON, false);
    std::ostringstream oss;
    formatter.print(results_, oss);

    std::string output = oss.str();
    EXPECT_NE(output.find("\"count\": 3"), std::string::npos);
    EXPECT_NE(output.find("\"files\":"), std::string::npos);
    EXPECT_NE(output.find("\"path\":"), std::string::npos);
    EXPECT_NE(output.find("\"name\":"), std::string::npos);
    EXPECT_NE(output.find("\"type\":"), std::string::npos);
    EXPECT_NE(output.find("\"size\":"), std::string::npos);
    EXPECT_NE(output.find("\"modified\":"), std::string::npos);
    EXPECT_NE(output.find("file1.txt"), std::string::npos);
    EXPECT_NE(output.find("\"type\": \"file\""), std::string::npos);
    EXPECT_NE(output.find("\"type\": \"directory\""), std::string::npos);
}

TEST_F(OutputFormatterTest, EmptyResults)
{
    SearchResult emptyResults;
    OutputFormatter formatter;
    std::ostringstream oss;
    formatter.print(emptyResults, oss);

    std::string output = oss.str();
    EXPECT_NE(output.find("Found 0 items"), std::string::npos);
}

TEST_F(OutputFormatterTest, JSONFormatValidStructure)
{
    OutputFormatter formatter(OutputFormat::JSON, false);
    std::ostringstream oss;
    formatter.print(results_, oss);

    std::string output = oss.str();
    // Basic JSON structure validation
    EXPECT_EQ(output.front(), '{');
    EXPECT_EQ(output.back(), '\n');
    EXPECT_NE(output.find('{'), std::string::npos);
    EXPECT_NE(output.find('}'), std::string::npos);
    EXPECT_NE(output.find('['), std::string::npos);
    EXPECT_NE(output.find(']'), std::string::npos);
}
