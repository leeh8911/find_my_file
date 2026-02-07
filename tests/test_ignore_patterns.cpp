#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "ignore_patterns.h"

using namespace fmf;

class IgnorePatternsTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        // Create a temporary test directory
        testDir_ = std::filesystem::temp_directory_path() / "fmf_ignore_test";
        std::filesystem::create_directories(testDir_);

        // Create test ignore file
        ignoreFile_ = testDir_ / ".gitignore";
    }

    void TearDown() override
    {
        // Clean up test directory
        std::filesystem::remove_all(testDir_);
    }

    std::filesystem::path testDir_;
    std::filesystem::path ignoreFile_;
};

TEST_F(IgnorePatternsTest, DefaultConstructor)
{
    IgnorePatterns patterns;
    EXPECT_TRUE(patterns.empty());
    EXPECT_EQ(patterns.size(), 0);
}

TEST_F(IgnorePatternsTest, AddPattern)
{
    IgnorePatterns patterns;
    patterns.addPattern("*.log");
    patterns.addPattern("build/");

    EXPECT_FALSE(patterns.empty());
    EXPECT_EQ(patterns.size(), 2);
}

TEST_F(IgnorePatternsTest, IgnoreComments)
{
    IgnorePatterns patterns;
    patterns.addPattern("# This is a comment");
    patterns.addPattern("*.log");

    EXPECT_EQ(patterns.size(), 1);  // Only non-comment pattern
}

TEST_F(IgnorePatternsTest, IgnoreEmptyLines)
{
    IgnorePatterns patterns;
    patterns.addPattern("");
    patterns.addPattern("   ");
    patterns.addPattern("*.log");

    EXPECT_EQ(patterns.size(), 1);
}

TEST_F(IgnorePatternsTest, ShouldIgnoreWildcard)
{
    IgnorePatterns patterns;
    patterns.addPattern("*.log");
    patterns.addPattern("*.tmp");

    EXPECT_TRUE(patterns.shouldIgnore("test.log"));
    EXPECT_TRUE(patterns.shouldIgnore("error.tmp"));
    EXPECT_FALSE(patterns.shouldIgnore("test.txt"));
}

TEST_F(IgnorePatternsTest, ShouldIgnoreDirectory)
{
    IgnorePatterns patterns;
    patterns.addPattern("build/");

    std::filesystem::path buildDir = testDir_ / "build";
    std::filesystem::create_directory(buildDir);

    EXPECT_TRUE(patterns.shouldIgnore(buildDir));
}

TEST_F(IgnorePatternsTest, LoadFromFile)
{
    // Create ignore file
    std::ofstream ofs(ignoreFile_);
    ofs << "# Comment line\n";
    ofs << "*.log\n";
    ofs << "build/\n";
    ofs << "\n";  // Empty line
    ofs << "temp*\n";
    ofs.close();

    IgnorePatterns patterns;
    EXPECT_TRUE(patterns.loadFromFile(ignoreFile_));
    EXPECT_EQ(patterns.size(), 3);  // Excluding comment and empty line
}

TEST_F(IgnorePatternsTest, LoadFromNonExistentFile)
{
    IgnorePatterns patterns;
    std::filesystem::path nonExistent = testDir_ / "does_not_exist";
    EXPECT_FALSE(patterns.loadFromFile(nonExistent));
    EXPECT_TRUE(patterns.empty());
}

TEST_F(IgnorePatternsTest, Clear)
{
    IgnorePatterns patterns;
    patterns.addPattern("*.log");
    patterns.addPattern("*.tmp");

    EXPECT_EQ(patterns.size(), 2);

    patterns.clear();
    EXPECT_TRUE(patterns.empty());
    EXPECT_EQ(patterns.size(), 0);
}

TEST_F(IgnorePatternsTest, MatchFileName)
{
    IgnorePatterns patterns;
    patterns.addPattern("test*");

    EXPECT_TRUE(patterns.shouldIgnore("test.txt"));
    EXPECT_TRUE(patterns.shouldIgnore("test_file.cpp"));
    EXPECT_FALSE(patterns.shouldIgnore("myfile.txt"));
}
