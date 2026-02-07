/**
 * @file local_embedding_provider.cpp
 * @brief Implementation of LocalEmbeddingProvider
 */

#include "infrastructure/ai/local_embedding_provider.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

namespace fmf
{

/**
 * @brief Implementation details (Pimpl idiom)
 *
 * Hides ONNX Runtime dependencies from header.
 * TODO: Integrate ONNX Runtime session and tensors.
 */
class LocalEmbeddingProvider::Impl
{
 public:
    explicit Impl(const std::string& modelPath, bool useGpu)
        : m_modelPath(modelPath), m_useGpu(useGpu), m_dimension(384)
    {
        // TODO: Initialize ONNX Runtime session
        // For now, this is a placeholder implementation
        initializeModel();
    }

    std::vector<float> infer(const std::vector<int64_t>& tokens)
    {
        // TODO: Replace with actual ONNX inference
        // For now, return deterministic "fake" embedding based on tokens

        std::vector<float> embedding(m_dimension);

        // Simple hash-based embedding (deterministic for testing)
        size_t hash = 0;
        for (auto token : tokens)
        {
            hash = hash * 31 + static_cast<size_t>(token);
        }

        std::mt19937 gen(hash);  // Deterministic random
        std::normal_distribution<float> dist(0.0f, 1.0f);

        for (size_t i = 0; i < m_dimension; ++i)
        {
            embedding[i] = dist(gen);
        }

        // Normalize to unit vector (cosine similarity friendly)
        normalizeVector(embedding);

        return embedding;
    }

    std::vector<std::vector<float>> inferBatch(
        const std::vector<std::vector<int64_t>>& batchTokens)
    {
        // TODO: Replace with actual ONNX batch inference
        std::vector<std::vector<float>> embeddings;
        embeddings.reserve(batchTokens.size());

        for (const auto& tokens : batchTokens)
        {
            embeddings.push_back(infer(tokens));
        }

        return embeddings;
    }

    size_t getDimension() const
    {
        return m_dimension;
    }

 private:
    void initializeModel()
    {
        // TODO: Load ONNX model and create inference session
        // For now, just verify model file exists
        if (!std::filesystem::exists(m_modelPath))
        {
            throw std::runtime_error("Model file not found: " + m_modelPath);
        }
    }

    void normalizeVector(std::vector<float>& vec) const
    {
        float norm = 0.0f;
        for (float val : vec)
        {
            norm += val * val;
        }
        norm = std::sqrt(norm);

        if (norm > 1e-6f)
        {
            for (float& val : vec)
            {
                val /= norm;
            }
        }
    }

    std::string m_modelPath;
    bool m_useGpu;
    size_t m_dimension;
    // TODO: Add ONNX session member
    // Ort::Session m_session;
};

LocalEmbeddingProvider::LocalEmbeddingProvider(const std::string& modelPath,
                                               bool useGpu)
    : m_impl(std::make_unique<Impl>(resolveModelPath(modelPath), useGpu))
{
}

LocalEmbeddingProvider::~LocalEmbeddingProvider() = default;

std::vector<float> LocalEmbeddingProvider::generateEmbedding(
    const std::string& text)
{
    auto tokens = tokenize(text);
    return m_impl->infer(tokens);
}

std::vector<std::vector<float>> LocalEmbeddingProvider::batchGenerate(
    const std::vector<std::string>& texts)
{
    std::vector<std::vector<int64_t>> batchTokens;
    batchTokens.reserve(texts.size());

    for (const auto& text : texts)
    {
        batchTokens.push_back(tokenize(text));
    }

    return m_impl->inferBatch(batchTokens);
}

size_t LocalEmbeddingProvider::getDimension() const
{
    return m_impl->getDimension();
}

std::vector<int64_t> LocalEmbeddingProvider::tokenize(
    const std::string& text) const
{
    // TODO: Implement proper BPE/WordPiece tokenizer
    // For now, use simple whitespace tokenization with character hashing

    std::vector<int64_t> tokens;
    std::istringstream stream(text);
    std::string word;

    while (stream >> word)
    {
        // Simple hash function for words
        int64_t hash = 0;
        for (char c : word)
        {
            hash = hash * 31 + static_cast<int64_t>(c);
        }
        tokens.push_back(std::abs(hash) % 30000);  // Vocab size ~30k
    }

    // Add special tokens (CLS, SEP)
    if (!tokens.empty())
    {
        tokens.insert(tokens.begin(), 101);  // [CLS] token
        tokens.push_back(102);               // [SEP] token
    }

    return tokens;
}

std::string LocalEmbeddingProvider::resolveModelPath(
    const std::string& modelPath) const
{
    namespace fs = std::filesystem;

    // If absolute path, use as-is
    if (fs::path(modelPath).is_absolute())
    {
        return modelPath;
    }

    // Check ~/.fmf/models/
    const char* home = std::getenv("HOME");
    if (home)
    {
        fs::path modelDir = fs::path(home) / ".fmf" / "models" / modelPath;
        if (fs::exists(modelDir))
        {
            return modelDir.string();
        }
    }

    // Check current directory
    if (fs::exists(modelPath))
    {
        return fs::absolute(modelPath).string();
    }

    // Not found - return original path (will fail in Impl constructor)
    return modelPath;
}

}  // namespace fmf
