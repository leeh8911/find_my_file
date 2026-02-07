#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "content_searcher.h"

using namespace fmf;

class ContentSearcherTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        // Create a temporary test directory
        testDir_ = std::filesystem::temp_directory_path() / "fmf_content_test";
        std::filesystem::create_directories(testDir_);

        // Create test files
        textFile_ = testDir_ / "test.txt";
        std::ofstream ofs(textFile_);
        ofs << "Hello World\n";
        ofs << "This is a test file\n";
        ofs << "Contains multiple lines\n";
        ofs.close();

        binaryFile_ = testDir_ / "test.bin";
        std::ofstream bofs(binaryFile_, std::ios::binary);
        for (int i = 0; i < 100; ++i)
        {
            bofs.put(static_cast<char>(i));
        }
        bofs.close();
    }

    void TearDown() override
    {
        // Clean up test directory
        std::filesystem::remove_all(testDir_);
    }

    std::filesystem::path testDir_;
    std::filesystem::path textFile_;
    std::filesystem::path binaryFile_;
};

TEST_F(ContentSearcherTest, SearchInTextFile)
{
    EXPECT_TRUE(ContentSearcher::searchInFile(textFile_, "Hello", false, true));
    EXPECT_TRUE(ContentSearcher::searchInFile(textFile_, "test", false, true));
    EXPECT_FALSE(
        ContentSearcher::searchInFile(textFile_, "notfound", false, true));
}

TEST_F(ContentSearcherTest, CaseInsensitiveSearch)
{
    EXPECT_TRUE(
        ContentSearcher::searchInFile(textFile_, "hello", false, false));
    EXPECT_TRUE(
        ContentSearcher::searchInFile(textFile_, "WORLD", false, false));
}

TEST_F(ContentSearcherTest, RegexSearch)
{
    EXPECT_TRUE(ContentSearcher::searchInFile(textFile_, "H.*o", true, true));
    EXPECT_TRUE(
        ContentSearcher::searchInFile(textFile_, "test.*file", true, true));
    EXPECT_FALSE(
        ContentSearcher::searchInFile(textFile_, "^notfound", true, true));
}

TEST_F(ContentSearcherTest, IsTextFile)
{
    EXPECT_TRUE(ContentSearcher::isTextFile(textFile_));
    EXPECT_FALSE(ContentSearcher::isTextFile(binaryFile_));
}

TEST_F(ContentSearcherTest, SearchInNonExistentFile)
{
    std::filesystem::path nonExistent = testDir_ / "does_not_exist.txt";
    EXPECT_FALSE(
        ContentSearcher::searchInFile(nonExistent, "test", false, true));
}

TEST_F(ContentSearcherTest, SearchInBinaryFile)
{
    // Binary files should not be searched
    EXPECT_FALSE(
        ContentSearcher::searchInFile(binaryFile_, "test", false, true));
}
