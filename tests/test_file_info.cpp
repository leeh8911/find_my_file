#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "file_info.h"

using namespace fmf;

class FileInfoTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        // Create a temporary test directory
        testDir_ = std::filesystem::temp_directory_path() / "fmf_test";
        std::filesystem::create_directories(testDir_);

        // Create test files
        testFile_ = testDir_ / "test_file.txt";
        std::ofstream ofs(testFile_);
        ofs << "Hello, World!";
        ofs.close();

        testSubDir_ = testDir_ / "subdir";
        std::filesystem::create_directory(testSubDir_);
    }

    void TearDown() override
    {
        // Clean up test directory
        std::filesystem::remove_all(testDir_);
    }

    std::filesystem::path testDir_;
    std::filesystem::path testFile_;
    std::filesystem::path testSubDir_;
};

TEST_F(FileInfoTest, ConstructorWithValidFile)
{
    ASSERT_NO_THROW({
        FileInfo fileInfo(testFile_);
        EXPECT_EQ(fileInfo.getFileName(), "test_file.txt");
        EXPECT_EQ(fileInfo.getExtension(), ".txt");
        EXPECT_TRUE(fileInfo.isRegularFile());
        EXPECT_FALSE(fileInfo.isDirectory());
    });
}

TEST_F(FileInfoTest, ConstructorWithDirectory)
{
    ASSERT_NO_THROW({
        FileInfo dirInfo(testSubDir_);
        EXPECT_EQ(dirInfo.getFileName(), "subdir");
        EXPECT_TRUE(dirInfo.isDirectory());
        EXPECT_FALSE(dirInfo.isRegularFile());
    });
}

TEST_F(FileInfoTest, ConstructorWithNonExistentFile)
{
    std::filesystem::path nonExistent = testDir_ / "does_not_exist.txt";
    EXPECT_THROW({ FileInfo fileInfo(nonExistent); }, std::runtime_error);
}

TEST_F(FileInfoTest, GetFileSize)
{
    FileInfo fileInfo(testFile_);
    EXPECT_GT(fileInfo.getSize(), 0);
}

TEST_F(FileInfoTest, GetPath)
{
    FileInfo fileInfo(testFile_);
    EXPECT_EQ(fileInfo.getPath(), testFile_.string());
}

TEST_F(FileInfoTest, GetLastWriteTime)
{
    FileInfo fileInfo(testFile_);
    // Just check that we can get the time without throwing
    EXPECT_NO_THROW(
        { [[maybe_unused]] auto time = fileInfo.getLastWriteTime(); });
}
