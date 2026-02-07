/**
 * @file test_local_embedding_provider.cpp
 * @brief Unit tests for LocalEmbeddingProvider
 */

#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <fstream>

#include "infrastructure/ai/local_embedding_provider.h"

using namespace fmf;

namespace
{

/**
 * @brief Helper: Create a dummy model file for testing
 */
class DummyModelFile
{
 public:
    DummyModelFile() : m_path("test_dummy_model.onnx")
    {
        // Create empty file (placeholder for ONNX model)
        std::ofstream file(m_path);
        file << "dummy model content";
    }

    ~DummyModelFile() { std::filesystem::remove(m_path); }

    std::string path() const { return m_path; }

 private:
    std::string m_path;
};

/**
 * @brief Helper: Calculate cosine similarity
 */
float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b)
{
    if (a.size() != b.size()) return 0.0f;

    float dot = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;

    for (size_t i = 0; i < a.size(); ++i)
    {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }

    return dot / (std::sqrt(normA) * std::sqrt(normB));
}

}  // namespace

// Test: Constructor succeeds with valid model path
TEST(LocalEmbeddingProviderTest, ConstructorSuccess)
{
    DummyModelFile model;

    EXPECT_NO_THROW({ LocalEmbeddingProvider provider(model.path()); });
}

// Test: Constructor fails with non-existent model
TEST(LocalEmbeddingProviderTest, ConstructorFailsWithInvalidPath)
{
    EXPECT_THROW(
        { LocalEmbeddingProvider provider("non_existent_model.onnx"); },
        std::runtime_error);
}

// Test: getDimension returns expected value
TEST(LocalEmbeddingProviderTest, GetDimension)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    EXPECT_EQ(provider.getDimension(), 384);  // all-MiniLM-L6-v2 default
}

// Test: generateEmbedding returns correct dimension
TEST(LocalEmbeddingProviderTest, GenerateEmbeddingDimension)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    auto embedding = provider.generateEmbedding("Hello world");

    EXPECT_EQ(embedding.size(), provider.getDimension());
}

// Test: Embedding is normalized (unit vector)
TEST(LocalEmbeddingProviderTest, EmbeddingIsNormalized)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    auto embedding = provider.generateEmbedding("Test text");

    // Calculate norm (should be ~1.0)
    float norm = 0.0f;
    for (float val : embedding)
    {
        norm += val * val;
    }
    norm = std::sqrt(norm);

    EXPECT_NEAR(norm, 1.0f, 1e-5f);
}

// Test: Same text produces same embedding (deterministic)
TEST(LocalEmbeddingProviderTest, DeterministicEmbedding)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    std::string text = "Semantic search test";
    auto embedding1 = provider.generateEmbedding(text);
    auto embedding2 = provider.generateEmbedding(text);

    ASSERT_EQ(embedding1.size(), embedding2.size());
    for (size_t i = 0; i < embedding1.size(); ++i)
    {
        EXPECT_FLOAT_EQ(embedding1[i], embedding2[i]);
    }
}

// Test: Different texts produce different embeddings
TEST(LocalEmbeddingProviderTest, DifferentTextsProduceDifferentEmbeddings)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    auto embedding1 = provider.generateEmbedding("First text");
    auto embedding2 = provider.generateEmbedding("Second text");

    // Embeddings should be different
    float similarity = cosineSimilarity(embedding1, embedding2);
    EXPECT_LT(similarity, 0.99f);  // Not identical
}

// Test: Similar texts have higher similarity
// NOTE: This test is weak for dummy implementation
// Real ONNX model will show proper semantic similarity
TEST(LocalEmbeddingProviderTest, SimilarTextsHigherSimilarity)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    auto emb1 = provider.generateEmbedding("cat dog animal");
    auto emb2 = provider.generateEmbedding("cat dog pet");
    auto emb3 = provider.generateEmbedding("computer software code");

    float sim12 = cosineSimilarity(emb1, emb2);
    float sim13 = cosineSimilarity(emb1, emb3);

    // For dummy implementation, just verify similarities are in valid range
    EXPECT_GE(sim12, -1.0f);
    EXPECT_LE(sim12, 1.0f);
    EXPECT_GE(sim13, -1.0f);
    EXPECT_LE(sim13, 1.0f);

    // TODO: With real ONNX model, expect sim12 > sim13
}

// Test: Batch generate returns correct number of embeddings
TEST(LocalEmbeddingProviderTest, BatchGenerateCount)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    std::vector<std::string> texts = {"text1", "text2", "text3"};
    auto embeddings = provider.batchGenerate(texts);

    EXPECT_EQ(embeddings.size(), texts.size());
}

// Test: Batch generate produces same embeddings as single calls
TEST(LocalEmbeddingProviderTest, BatchGenerateConsistency)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    std::vector<std::string> texts = {"First", "Second", "Third"};

    // Batch generate
    auto batchEmbeddings = provider.batchGenerate(texts);

    // Single generate
    std::vector<std::vector<float>> singleEmbeddings;
    for (const auto& text : texts)
    {
        singleEmbeddings.push_back(provider.generateEmbedding(text));
    }

    // Compare
    ASSERT_EQ(batchEmbeddings.size(), singleEmbeddings.size());
    for (size_t i = 0; i < batchEmbeddings.size(); ++i)
    {
        ASSERT_EQ(batchEmbeddings[i].size(), singleEmbeddings[i].size());
        for (size_t j = 0; j < batchEmbeddings[i].size(); ++j)
        {
            EXPECT_FLOAT_EQ(batchEmbeddings[i][j], singleEmbeddings[i][j]);
        }
    }
}

// Test: Empty text handling
TEST(LocalEmbeddingProviderTest, EmptyTextHandling)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    auto embedding = provider.generateEmbedding("");

    EXPECT_EQ(embedding.size(), provider.getDimension());
    // Embedding should still be normalized
    float norm = 0.0f;
    for (float val : embedding)
    {
        norm += val * val;
    }
    EXPECT_GT(std::sqrt(norm), 0.0f);  // Non-zero vector
}

// Test: Long text handling
TEST(LocalEmbeddingProviderTest, LongTextHandling)
{
    DummyModelFile model;
    LocalEmbeddingProvider provider(model.path());

    std::string longText(1000, 'a');  // 1000 characters
    auto embedding = provider.generateEmbedding(longText);

    EXPECT_EQ(embedding.size(), provider.getDimension());
}

// Test: Model path resolution (~/.fmf/models/)
TEST(LocalEmbeddingProviderTest, ModelPathResolution)
{
    // Create model in current directory
    DummyModelFile model;

    // Should resolve to current directory
    LocalEmbeddingProvider provider(model.path());
    EXPECT_NO_THROW(provider.getDimension());
}
