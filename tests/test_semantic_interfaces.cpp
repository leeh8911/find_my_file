/**
 * @file test_semantic_interfaces.cpp
 * @brief Unit tests for semantic search interfaces
 *
 * Tests for:
 * - IEmbeddingProvider interface
 * - IVectorStore interface
 * - SemanticResult structure
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <vector>

#include "i_embedding_provider.h"
#include "i_vector_store.h"
#include "semantic_result.h"

namespace fmf
{

/**
 * @brief Mock implementation of IEmbeddingProvider for testing
 */
class MockEmbeddingProvider : public IEmbeddingProvider
{
 public:
    std::vector<float> generateEmbedding(const std::string& text) override
    {
        // Return a simple deterministic vector based on text length
        size_t dim = getDimension();
        std::vector<float> embedding(dim, 0.0f);
        
        // Simple hash: use text length as feature
        if (!text.empty())
        {
            embedding[0] = static_cast<float>(text.length()) / 100.0f;
        }
        
        return embedding;
    }

    std::vector<std::vector<float>> batchGenerate(
        const std::vector<std::string>& texts) override
    {
        std::vector<std::vector<float>> results;
        for (const auto& text : texts)
        {
            results.push_back(generateEmbedding(text));
        }
        return results;
    }

    size_t getDimension() const override { return 128; }
};

/**
 * @brief Mock implementation of IVectorStore for testing
 */
class MockVectorStore : public IVectorStore
{
 public:
    void add(const std::string& id, const std::vector<float>& vector,
             const VectorMetadata& metadata) override
    {
        vectors_[id] = vector;
        metadata_[id] = metadata;
    }

    std::vector<SemanticResult> search(const std::vector<float>& queryVector,
                                       size_t topK) override
    {
        std::vector<SemanticResult> results;
        
        // Simple cosine similarity search
        for (const auto& [id, vector] : vectors_)
        {
            float similarity = computeSimilarity(queryVector, vector);
            
            FileInfo fileInfo(metadata_[id].filepath);
            SemanticResult result(fileInfo, similarity);
            result.matchedChunks.push_back(metadata_[id].chunkText);
            
            results.push_back(result);
        }
        
        // Sort by score and return top-K
        std::sort(results.begin(), results.end(),
                  [](const SemanticResult& a, const SemanticResult& b) {
                      return a.relevanceScore > b.relevanceScore;
                  });
        
        if (results.size() > topK)
        {
            results.resize(topK);
        }
        
        return results;
    }

    bool remove(const std::string& id) override
    {
        auto it = vectors_.find(id);
        if (it != vectors_.end())
        {
            vectors_.erase(it);
            metadata_.erase(id);
            return true;
        }
        return false;
    }

    bool save(const std::string& path) override
    {
        // Mock: always succeed
        (void)path;  // Mark as unused
        return true;
    }

    bool load(const std::string& path) override
    {
        // Mock: always succeed
        (void)path;  // Mark as unused
        return true;
    }

    size_t size() const override { return vectors_.size(); }

    void clear() override
    {
        vectors_.clear();
        metadata_.clear();
    }

 private:
    float computeSimilarity(const std::vector<float>& a,
                            const std::vector<float>& b)
    {
        if (a.size() != b.size())
            return 0.0f;
        
        float dot = 0.0f;
        float mag_a = 0.0f;
        float mag_b = 0.0f;
        
        for (size_t i = 0; i < a.size(); ++i)
        {
            dot += a[i] * b[i];
            mag_a += a[i] * a[i];
            mag_b += b[i] * b[i];
        }
        
        mag_a = std::sqrt(mag_a);
        mag_b = std::sqrt(mag_b);
        
        if (mag_a == 0.0f || mag_b == 0.0f)
            return 0.0f;
        
        return dot / (mag_a * mag_b);
    }

    std::map<std::string, std::vector<float>> vectors_;
    std::map<std::string, VectorMetadata> metadata_;
};

/**
 * @brief Test fixture for semantic interfaces
 */
class SemanticInterfacesTest : public ::testing::Test
{
 protected:
    void SetUp() override
    {
        provider = std::make_unique<MockEmbeddingProvider>();
        store = std::make_unique<MockVectorStore>();
    }

    void TearDown() override {}

    std::unique_ptr<IEmbeddingProvider> provider;
    std::unique_ptr<IVectorStore> store;
};

/**
 * @brief Test: IEmbeddingProvider interface exists
 */
TEST_F(SemanticInterfacesTest, EmbeddingProviderInterface)
{
    EXPECT_TRUE(provider != nullptr);
    EXPECT_EQ(provider->getDimension(), 128);
}

/**
 * @brief Test: IEmbeddingProvider generateEmbedding method
 */
TEST_F(SemanticInterfacesTest, EmbeddingProviderGenerate)
{
    std::string text = "Hello, world!";
    auto embedding = provider->generateEmbedding(text);
    
    EXPECT_EQ(embedding.size(), 128);
    EXPECT_GT(embedding[0], 0.0f);  // Should have some value based on text
}

/**
 * @brief Test: IEmbeddingProvider batchGenerate method
 */
TEST_F(SemanticInterfacesTest, EmbeddingProviderBatchGenerate)
{
    std::vector<std::string> texts = {"text1", "text2", "text3"};
    auto embeddings = provider->batchGenerate(texts);
    
    EXPECT_EQ(embeddings.size(), 3);
    for (const auto& emb : embeddings)
    {
        EXPECT_EQ(emb.size(), 128);
    }
}

/**
 * @brief Test: IEmbeddingProvider getDimension method
 */
TEST_F(SemanticInterfacesTest, EmbeddingProviderGetDimension)
{
    size_t dim = provider->getDimension();
    EXPECT_EQ(dim, 128);
}

/**
 * @brief Test: IVectorStore interface exists
 */
TEST_F(SemanticInterfacesTest, VectorStoreInterface)
{
    EXPECT_TRUE(store != nullptr);
    EXPECT_EQ(store->size(), 0);
}

/**
 * @brief Test: IVectorStore add method
 */
TEST_F(SemanticInterfacesTest, VectorStoreAdd)
{
    std::vector<float> vector(128, 1.0f);
    VectorMetadata metadata;
    metadata.filepath = "test.txt";
    metadata.chunkText = "test content";
    
    store->add("id1", vector, metadata);
    
    EXPECT_EQ(store->size(), 1);
}

/**
 * @brief Test: IVectorStore search method
 */
TEST_F(SemanticInterfacesTest, VectorStoreSearch)
{
    // Create temporary test files
    std::ofstream file1("/tmp/fmf_test_file1.txt");
    file1 << "test content 1";
    file1.close();
    
    std::ofstream file2("/tmp/fmf_test_file2.txt");
    file2 << "test content 2";
    file2.close();
    
    // Add some vectors
    std::vector<float> vec1(128, 1.0f);
    VectorMetadata meta1;
    meta1.filepath = "/tmp/fmf_test_file1.txt";
    meta1.chunkText = "content 1";
    store->add("id1", vec1, meta1);
    
    std::vector<float> vec2(128, 0.5f);
    VectorMetadata meta2;
    meta2.filepath = "/tmp/fmf_test_file2.txt";
    meta2.chunkText = "content 2";
    store->add("id2", vec2, meta2);
    
    // Search
    std::vector<float> query(128, 0.9f);
    auto results = store->search(query, 1);
    
    EXPECT_EQ(results.size(), 1);
    EXPECT_GT(results[0].relevanceScore, 0.0f);
    
    // Cleanup
    std::filesystem::remove("/tmp/fmf_test_file1.txt");
    std::filesystem::remove("/tmp/fmf_test_file2.txt");
}

/**
 * @brief Test: IVectorStore remove method
 */
TEST_F(SemanticInterfacesTest, VectorStoreRemove)
{
    std::vector<float> vector(128, 1.0f);
    VectorMetadata metadata;
    metadata.filepath = "test.txt";
    
    store->add("id1", vector, metadata);
    EXPECT_EQ(store->size(), 1);
    
    bool removed = store->remove("id1");
    EXPECT_TRUE(removed);
    EXPECT_EQ(store->size(), 0);
    
    bool removed_again = store->remove("id1");
    EXPECT_FALSE(removed_again);
}

/**
 * @brief Test: IVectorStore save/load methods
 */
TEST_F(SemanticInterfacesTest, VectorStorePersistence)
{
    bool saved = store->save("/tmp/test_index");
    EXPECT_TRUE(saved);
    
    bool loaded = store->load("/tmp/test_index");
    EXPECT_TRUE(loaded);
}

/**
 * @brief Test: SemanticResult structure
 */
TEST_F(SemanticInterfacesTest, SemanticResultStructure)
{
    // Create temporary test file
    std::ofstream file("/tmp/fmf_test.txt");
    file << "test content";
    file.close();
    
    FileInfo fileInfo("/tmp/fmf_test.txt");
    SemanticResult result(fileInfo, 0.85f);
    
    EXPECT_EQ(result.fileInfo.getFileName(), "fmf_test.txt");
    EXPECT_FLOAT_EQ(result.relevanceScore, 0.85f);
    EXPECT_TRUE(result.matchedChunks.empty());
    
    // Cleanup
    std::filesystem::remove("/tmp/fmf_test.txt");
}

/**
 * @brief Test: SemanticResult with valid data
 */
TEST_F(SemanticInterfacesTest, SemanticResultWithData)
{
    // Create temporary test file
    std::ofstream file("/tmp/fmf_test2.txt");
    file << "test content";
    file.close();
    
    FileInfo fileInfo("/tmp/fmf_test2.txt");
    std::vector<std::string> chunks = {"chunk1", "chunk2"};
    SemanticResult result(fileInfo, 0.92f, chunks);
    
    EXPECT_EQ(result.fileInfo.getFileName(), "fmf_test2.txt");
    EXPECT_FLOAT_EQ(result.relevanceScore, 0.92f);
    EXPECT_EQ(result.matchedChunks.size(), 2);
    EXPECT_EQ(result.matchedChunks[0], "chunk1");
    EXPECT_EQ(result.matchedChunks[1], "chunk2");
    
    // Cleanup
    std::filesystem::remove("/tmp/fmf_test2.txt");
}

}  // namespace fmf
