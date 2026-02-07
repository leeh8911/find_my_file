/**
 * @file local_embedding_provider.h
 * @brief Local embedding provider using ONNX Runtime
 *
 * Provides text embeddings using local ML models (ONNX format).
 * Supports Sentence-BERT models like all-MiniLM-L6-v2.
 *
 * SOLID Principles:
 * - SRP: Single responsibility - local model inference
 * - DIP: Implements IEmbeddingProvider abstraction
 * - OCP: Extensible for different ONNX models
 */

#ifndef FMF_LOCAL_EMBEDDING_PROVIDER_H
#define FMF_LOCAL_EMBEDDING_PROVIDER_H

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "application/ports/i_embedding_provider.h"

namespace fmf
{

/**
 * @brief Local embedding provider using ONNX Runtime
 *
 * Features:
 * - Runs entirely offline (no API calls)
 * - Supports ONNX format models
 * - Default: all-MiniLM-L6-v2 (384 dimensions)
 * - CPU optimized with optional BLAS/OpenMP
 *
 * Model Management:
 * - Models stored in ~/.fmf/models/
 * - Auto-download on first use (optional)
 * - SHA256 verification for security
 *
 * Example:
 * @code
 * LocalEmbeddingProvider provider("all-MiniLM-L6-v2.onnx");
 * auto embedding = provider.generateEmbedding("Hello world");
 * std::cout << "Dimension: " << provider.getDimension() << std::endl;
 * @endcode
 */
class LocalEmbeddingProvider : public IEmbeddingProvider
{
 public:
    /**
     * @brief Construct provider with model path
     *
     * @param modelPath Path to ONNX model file
     *                  If relative, searches in ~/.fmf/models/
     * @param useGpu Enable GPU acceleration (requires CUDA/ROCm)
     *
     * @throws std::runtime_error if model file not found or invalid
     */
    explicit LocalEmbeddingProvider(const std::string& modelPath,
                                     bool useGpu = false);

    ~LocalEmbeddingProvider() override;

    // Disable copy (ONNX session is not copyable)
    LocalEmbeddingProvider(const LocalEmbeddingProvider&) = delete;
    LocalEmbeddingProvider& operator=(const LocalEmbeddingProvider&) = delete;

    // Enable move
    LocalEmbeddingProvider(LocalEmbeddingProvider&&) noexcept = default;
    LocalEmbeddingProvider& operator=(LocalEmbeddingProvider&&) noexcept =
        default;

    /**
     * @brief Generate embedding for a single text
     *
     * @param text Input text (max 512 tokens recommended)
     * @return Embedding vector (dimension: 384 for MiniLM)
     *
     * @throws std::runtime_error on inference errors
     */
    std::vector<float> generateEmbedding(
        const std::string& text) override;

    /**
     * @brief Generate embeddings for multiple texts (batch)
     *
     * More efficient than multiple single calls.
     * Internally uses ONNX batch inference.
     *
     * @param texts Vector of input texts
     * @return Vector of embeddings (one per input)
     *
     * @throws std::runtime_error on inference errors
     */
    std::vector<std::vector<float>> batchGenerate(
        const std::vector<std::string>& texts) override;

    /**
     * @brief Get embedding dimension
     *
     * @return Embedding dimension (384 for all-MiniLM-L6-v2)
     */
    size_t getDimension() const override;

 private:
    class Impl;  // Forward declaration for Pimpl
    std::unique_ptr<Impl> m_impl;

    /**
     * @brief Tokenize text to input IDs
     *
     * Simple whitespace tokenizer for now.
     * TODO: Proper BPE/WordPiece tokenizer
     *
     * @param text Input text
     * @return Vector of token IDs
     */
    std::vector<int64_t> tokenize(const std::string& text) const;

    /**
     * @brief Resolve model path (check ~/.fmf/models/)
     *
     * @param modelPath User-provided path
     * @return Absolute path to model file
     *
     * @throws std::runtime_error if not found
     */
    std::string resolveModelPath(const std::string& modelPath) const;
};

}  // namespace fmf

#endif  // FMF_LOCAL_EMBEDDING_PROVIDER_H
