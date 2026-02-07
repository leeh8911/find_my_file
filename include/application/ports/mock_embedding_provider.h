#ifndef MOCK_EMBEDDING_PROVIDER_H
#define MOCK_EMBEDDING_PROVIDER_H

#include <string>
#include <vector>

#include "application/ports/i_embedding_provider.h"

namespace fmf
{

/**
 * @brief Mock implementation of IEmbeddingProvider for testing
 *
 * This class provides a deterministic, lightweight embedding provider
 * that can be used in unit tests without external dependencies.
 *
 * Features:
 * - Deterministic output (same input → same output)
 * - No network calls or external API dependencies
 * - Simple hash-based embedding generation
 * - Configurable dimension size
 *
 * @note This is NOT suitable for production use. Use OpenAIProvider
 *       or other real embedding providers for actual semantic search.
 *
 * Example usage:
 * @code
 *   MockEmbeddingProvider provider(128);
 *   auto embedding = provider.generateEmbedding("test text");
 *   // embedding is a deterministic 128-dimensional vector
 * @endcode
 */
class MockEmbeddingProvider : public IEmbeddingProvider
{
 public:
    /**
     * @brief Construct with specified dimension
     * @param dimension The dimension of embedding vectors (default: 128)
     */
    explicit MockEmbeddingProvider(size_t dimension = 128)
        : dimension_(dimension)
    {
    }

    /**
     * @brief Generate embedding vector from text
     *
     * Uses a simple deterministic algorithm based on text length.
     * This is only for testing purposes.
     *
     * @param text Input text to embed
     * @return Embedding vector of size dimension_
     */
    std::vector<float> generateEmbedding(const std::string& text) override
    {
        std::vector<float> embedding(dimension_, 0.0f);

        // Simple hash: use text length as feature
        if (!text.empty())
        {
            embedding[0] = static_cast<float>(text.length()) / 100.0f;

            // Add some variation based on first/last characters
            if (text.length() > 1)
            {
                embedding[1] = static_cast<float>(text[0]) / 255.0f;
                embedding[2] =
                    static_cast<float>(text[text.length() - 1]) / 255.0f;
            }
        }

        return embedding;
    }

    /**
     * @brief Generate embeddings for multiple texts
     *
     * @param texts Vector of input texts
     * @return Vector of embedding vectors
     */
    std::vector<std::vector<float>> batchGenerate(
        const std::vector<std::string>& texts) override
    {
        std::vector<std::vector<float>> results;
        results.reserve(texts.size());

        for (const auto& text : texts)
        {
            results.push_back(generateEmbedding(text));
        }

        return results;
    }

    /**
     * @brief Get the dimension of embedding vectors
     * @return Dimension size
     */
    size_t getDimension() const override { return dimension_; }

 private:
    size_t dimension_;  ///< Dimension of embedding vectors
};

}  // namespace fmf

#endif  // MOCK_EMBEDDING_PROVIDER_H
