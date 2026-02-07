#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "file_scanner.h"

using namespace fmf;

class FileScannerTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        // Create a temporary test directory structure
        testDir_ = std::filesystem::temp_directory_path() / "fmf_scanner_test";
        std::filesystem::create_directories(testDir_);

        // Create test files
        createTestFile(testDir_ / "file1.txt", "Content 1");
        createTestFile(testDir_ / "file2.cpp", "Content 2");

        // Create subdirectory with files
        auto subDir = testDir_ / "subdir";
        std::filesystem::create_directory(subDir);
        createTestFile(subDir / "file3.txt", "Content 3");
        createTestFile(subDir / "file4.h", "Content 4");

        // Create nested subdirectory
        auto nestedDir = subDir / "nested";
        std::filesystem::create_directory(nestedDir);
        createTestFile(nestedDir / "file5.txt", "Content 5");
    }

    void TearDown() override
    {
        // Clean up test directory
        std::filesystem::remove_all(testDir_);
    }

    void createTestFile(const std::filesystem::path& path,
                        const std::string& content)
    {
        std::ofstream ofs(path);
        ofs << content;
        ofs.close();
    }

    std::filesystem::path testDir_;
};

TEST_F(FileScannerTest, ScanDirectoryNonRecursive)
{
    FileScanner scanner;
    auto results = scanner.scanDirectory(testDir_);

    // Should find 2 files and 1 subdirectory at the top level
    EXPECT_EQ(results.size(), 3);
}

TEST_F(FileScannerTest, ScanDirectoryRecursive)
{
    FileScanner scanner;
    auto results = scanner.scanDirectoryRecursive(testDir_);

    // Should find all files and directories recursively
    // 2 top-level files + 1 subdir + 2 files in subdir + 1 nested dir + 1 file
    // in nested
    EXPECT_GE(results.size(), 5);
}

TEST_F(FileScannerTest, ScanWithMaxDepth)
{
    FileScanner scanner;
    auto results = scanner.scanDirectoryRecursive(testDir_, 1);

    // With depth 1, should not find files in nested subdirectory
    // Should find: 2 top files + 1 subdir + 2 files in subdir + 1 nested dir
    // (but not its contents)
    EXPECT_GE(results.size(), 3);
    EXPECT_LE(results.size(), 6);
}

TEST_F(FileScannerTest, ScanWithFilter)
{
    FileScanner scanner;

    // Filter for .txt files only
    auto results =
        scanner.scanWithFilter(testDir_, true, [](const FileInfo& info)
                               { return info.getExtension() == ".txt"; });

    // Should find 3 .txt files (file1.txt, file3.txt, file5.txt)
    EXPECT_EQ(results.size(), 3);
}

TEST_F(FileScannerTest, ScanNonExistentDirectory)
{
    FileScanner scanner;
    auto nonExistent = testDir_ / "does_not_exist";
    auto results = scanner.scanDirectory(nonExistent);

    EXPECT_TRUE(results.empty());
}

TEST_F(FileScannerTest, ScanFileInsteadOfDirectory)
{
    FileScanner scanner;
    auto file = testDir_ / "file1.txt";
    auto results = scanner.scanDirectory(file);

    EXPECT_TRUE(results.empty());
}
