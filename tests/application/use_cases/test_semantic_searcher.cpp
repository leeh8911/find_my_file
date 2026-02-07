/**
 * @file test_semantic_searcher.cpp
 * @brief Unit tests for SemanticSearcher
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>

#include "application/ports/mock_embedding_provider.h"
#include "application/ports/mock_vector_store.h"
#include "application/use_cases/semantic_searcher.h"

using namespace fmf;

/**
 * @brief Test fixture for SemanticSearcher
 */
class SemanticSearcherTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        // Create test directory
        testDir = "/tmp/fmf_semantic_test";
        std::filesystem::create_directories(testDir);

        // Create test files
        createTestFile("file1.txt", "This is a test file about programming.");
        createTestFile("file2.txt", "Python is a great language for AI.");
        createTestFile("file3.txt", "C++ is used for system programming.");

        // Create test provider and store
        provider = std::make_unique<MockEmbeddingProvider>(128);
        store = std::make_unique<MockVectorStore>();
    }

    void TearDown() override
    {
        // Cleanup test directory
        std::filesystem::remove_all(testDir);
    }

    void createTestFile(const std::string& filename, const std::string& content)
    {
        std::ofstream file(testDir + "/" + filename);
        file << content;
        file.close();
    }

    std::string testDir;
    std::unique_ptr<IEmbeddingProvider> provider;
    std::unique_ptr<IVectorStore> store;
};

/**
 * @brief Test: SemanticSearcher construction
 */
TEST_F(SemanticSearcherTest, Construction)
{
    SemanticSearcher searcher(std::move(provider), std::move(store));
    EXPECT_EQ(searcher.size(), 0);
}

/**
 * @brief Test: Index single file
 */
TEST_F(SemanticSearcherTest, IndexFile)
{
    auto testProvider = std::make_unique<MockEmbeddingProvider>(128);
    auto testStore = std::make_unique<MockVectorStore>();
    SemanticSearcher searcher(std::move(testProvider), std::move(testStore));

    std::string filepath = testDir + "/file1.txt";
    bool result = searcher.indexFile(filepath);

    EXPECT_TRUE(result);
    EXPECT_GT(searcher.size(), 0);
}

/**
 * @brief Test: Index directory
 */
TEST_F(SemanticSearcherTest, IndexDirectory)
{
    auto testProvider = std::make_unique<MockEmbeddingProvider>(128);
    auto testStore = std::make_unique<MockVectorStore>();
    SemanticSearcher searcher(std::move(testProvider), std::move(testStore));

    size_t indexed = searcher.indexDirectory(testDir);

    EXPECT_EQ(indexed, 3);  // 3 test files
    EXPECT_GT(searcher.size(), 0);
}

/**
 * @brief Test: Search by query
 */
TEST_F(SemanticSearcherTest, SearchByQuery)
{
    auto testProvider = std::make_unique<MockEmbeddingProvider>(128);
    auto testStore = std::make_unique<MockVectorStore>();
    SemanticSearcher searcher(std::move(testProvider), std::move(testStore));

    // Index the directory first
    searcher.indexDirectory(testDir);

    // Search
    auto results = searcher.search("programming language", 2);

    EXPECT_FALSE(results.empty());
    EXPECT_LE(results.size(), 2);

    // Results should be sorted by relevance
    if (results.size() > 1)
    {
        EXPECT_GE(results[0].relevanceScore, results[1].relevanceScore);
    }
}

/**
 * @brief Test: Find similar files
 */
TEST_F(SemanticSearcherTest, SearchSimilar)
{
    auto testProvider = std::make_unique<MockEmbeddingProvider>(128);
    auto testStore = std::make_unique<MockVectorStore>();
    SemanticSearcher searcher(std::move(testProvider), std::move(testStore));

    // Index the directory first
    searcher.indexDirectory(testDir);

    // Find files similar to file1.txt
    std::string filepath = testDir + "/file1.txt";
    auto results = searcher.searchSimilar(filepath, 2);

    EXPECT_FALSE(results.empty());
    EXPECT_LE(results.size(), 2);
}
