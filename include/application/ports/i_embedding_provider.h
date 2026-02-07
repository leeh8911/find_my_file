/**
 * @file i_embedding_provider.h
 * @brief Interface for embedding providers
 *
 * Defines abstract interface for converting text to vector embeddings.
 * Implementations can use OpenAI API, local models, or mock providers.
 *
 * SOLID Principles:
 * - ISP (Interface Segregation): Minimal, focused interface
 * - DIP (Dependency Inversion): High-level modules depend on this abstraction
 */

#ifndef FMF_I_EMBEDDING_PROVIDER_H
#define FMF_I_EMBEDDING_PROVIDER_H

#pragma once

#include <string>
#include <vector>

namespace fmf
{

/**
 * @brief Abstract interface for embedding providers
 *
 * Providers transform text into fixed-dimension vector representations.
 * Different implementations can use different embedding models.
 *
 * Example implementations:
 * - OpenAIProvider: Uses OpenAI's text-embedding-ada-002
 * - LocalModelProvider: Uses ONNX or llama.cpp models
 * - MockProvider: Returns deterministic vectors for testing
 */
class IEmbeddingProvider
{
 public:
    virtual ~IEmbeddingProvider() = default;

    /**
     * @brief Generate embedding for a single text
     *
     * @param text Input text to embed
     * @return Vector representation (dimension determined by model)
     *
     * @throws std::runtime_error on API/model errors
     */
    virtual std::vector<float> generateEmbedding(
        const std::string& text) = 0;

    /**
     * @brief Generate embeddings for multiple texts (batch)
     *
     * More efficient than calling generateEmbedding multiple times.
     * Implementations can use batch API calls or parallel processing.
     *
     * @param texts Vector of input texts
     * @return Vector of embeddings (one per input text)
     *
     * @throws std::runtime_error on API/model errors
     */
    virtual std::vector<std::vector<float>> batchGenerate(
        const std::vector<std::string>& texts) = 0;

    /**
     * @brief Get embedding dimension
     *
     * All vectors from this provider have this fixed dimension.
     *
     * @return Embedding dimension (e.g., 1536 for ada-002)
     */
    virtual size_t getDimension() const = 0;
};

}  // namespace fmf

#endif  // FMF_I_EMBEDDING_PROVIDER_H
