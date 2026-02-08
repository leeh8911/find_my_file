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

#ifdef ENABLE_ONNX_RUNTIME
#include <onnxruntime_cxx_api.h>
#endif

#include "infrastructure/ai/simple_tokenizer.h"

namespace fmf
{

#ifdef ENABLE_ONNX_RUNTIME

/**
 * @brief ONNX Runtime implementation (real inference)
 */
class LocalEmbeddingProvider::Impl
{
 public:
    explicit Impl(const std::string& modelPath, bool useGpu)
        : m_modelPath(resolveModelPath(modelPath)),
          m_useGpu(useGpu),
          m_dimension(384),
          m_tokenizer(SimpleTokenizer::DEFAULT_MAX_LENGTH, true),
          m_env(ORT_LOGGING_LEVEL_WARNING, "LocalEmbeddingProvider")
    {
        initializeModel();
    }

    std::vector<float> infer(const std::vector<int64_t>& tokens)
    {
        // Prepare input tensors
        auto [inputIds, attentionMask] = prepareInputTensors({tokens});

        // Run inference
        auto outputs = runInference(inputIds, attentionMask);

        // Extract embedding (mean pooling)
        return extractEmbedding(outputs, attentionMask, 0);
    }

    std::vector<std::vector<float>> inferBatch(
        const std::vector<std::vector<int64_t>>& batchTokens)
    {
        // Prepare input tensors
        auto [inputIds, attentionMask] = prepareInputTensors(batchTokens);

        // Run inference
        auto outputs = runInference(inputIds, attentionMask);

        // Extract embeddings
        std::vector<std::vector<float>> embeddings;
        embeddings.reserve(batchTokens.size());

        for (size_t i = 0; i < batchTokens.size(); ++i)
        {
            embeddings.push_back(extractEmbedding(outputs, attentionMask, i));
        }

        return embeddings;
    }

    size_t getDimension() const { return m_dimension; }

    std::vector<int64_t> tokenize(const std::string& text) const
    {
        return m_tokenizer.tokenize(text);
    }

 private:
    void initializeModel()
    {
        if (!std::filesystem::exists(m_modelPath))
        {
            throw std::runtime_error("Model file not found: " + m_modelPath);
        }

        // Create session options
        m_sessionOptions.SetIntraOpNumThreads(1);
        m_sessionOptions.SetGraphOptimizationLevel(
            GraphOptimizationLevel::ORT_ENABLE_BASIC);

        if (m_useGpu)
        {
            // Enable CUDA provider if requested (requires CUDA)
            // OrtCUDAProviderOptions cuda_options;
            // m_sessionOptions.AppendExecutionProvider_CUDA(cuda_options);
        }

        // Create session and load model
#ifdef _WIN32
        std::wstring wideModelPath(m_modelPath.begin(), m_modelPath.end());
        m_session = std::make_unique<Ort::Session>(m_env, wideModelPath.c_str(),
                                                   m_sessionOptions);
#else
        m_session = std::make_unique<Ort::Session>(m_env, m_modelPath.c_str(),
                                                   m_sessionOptions);
#endif

        // Get input/output info
        m_inputNames.push_back("input_ids");
        m_inputNames.push_back("attention_mask");
        m_outputNames.push_back("last_hidden_state");
    }

    std::pair<std::vector<int64_t>, std::vector<int64_t>> prepareInputTensors(
        const std::vector<std::vector<int64_t>>& batchTokens)
    {
        size_t batchSize = batchTokens.size();
        size_t maxLength = m_tokenizer.getMaxLength();

        std::vector<int64_t> inputIds(batchSize * maxLength, 0);
        std::vector<int64_t> attentionMask(batchSize * maxLength, 0);

        for (size_t i = 0; i < batchSize; ++i)
        {
            const auto& tokens = batchTokens[i];
            size_t seqLen = std::min(tokens.size(), maxLength);

            for (size_t j = 0; j < seqLen; ++j)
            {
                inputIds[i * maxLength + j] = tokens[j];
                attentionMask[i * maxLength + j] = 1;
            }
        }

        return {inputIds, attentionMask};
    }

    std::vector<float> runInference(const std::vector<int64_t>& inputIds,
                                    const std::vector<int64_t>& attentionMask)
    {
        size_t batchSize = inputIds.size() / m_tokenizer.getMaxLength();
        size_t seqLength = m_tokenizer.getMaxLength();

        // Create input tensors
        Ort::MemoryInfo memoryInfo =
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        std::vector<int64_t> inputShape = {static_cast<int64_t>(batchSize),
                                           static_cast<int64_t>(seqLength)};

        Ort::Value inputIdsTensor = Ort::Value::CreateTensor<int64_t>(
            memoryInfo, const_cast<int64_t*>(inputIds.data()), inputIds.size(),
            inputShape.data(), inputShape.size());

        Ort::Value attentionMaskTensor = Ort::Value::CreateTensor<int64_t>(
            memoryInfo, const_cast<int64_t*>(attentionMask.data()),
            attentionMask.size(), inputShape.data(), inputShape.size());

        // Prepare input/output names
        std::vector<const char*> inputNamesChar;
        for (const auto& name : m_inputNames)
        {
            inputNamesChar.push_back(name.c_str());
        }

        std::vector<const char*> outputNamesChar;
        for (const auto& name : m_outputNames)
        {
            outputNamesChar.push_back(name.c_str());
        }

        // Run inference
        std::vector<Ort::Value> inputTensors;
        inputTensors.push_back(std::move(inputIdsTensor));
        inputTensors.push_back(std::move(attentionMaskTensor));

        auto outputTensors =
            m_session->Run(Ort::RunOptions{nullptr}, inputNamesChar.data(),
                           inputTensors.data(), inputTensors.size(),
                           outputNamesChar.data(), outputNamesChar.size());

        // Extract output data
        float* outputData = outputTensors[0].GetTensorMutableData<float>();
        auto outputShape =
            outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();

        size_t hiddenSize = outputShape[2];
        size_t totalElements = batchSize * seqLength * hiddenSize;

        return std::vector<float>(outputData, outputData + totalElements);
    }

    std::vector<float> extractEmbedding(
        const std::vector<float>& outputs,
        const std::vector<int64_t>& attentionMask, size_t batchIndex)
    {
        // Mean pooling: average all token embeddings (weighted by attention
        // mask)
        size_t seqLength = m_tokenizer.getMaxLength();
        size_t offset = batchIndex * seqLength * m_dimension;

        std::vector<float> embedding(m_dimension, 0.0f);
        int tokenCount = 0;

        for (size_t i = 0; i < seqLength; ++i)
        {
            if (attentionMask[batchIndex * seqLength + i] == 1)
            {
                for (size_t j = 0; j < m_dimension; ++j)
                {
                    embedding[j] += outputs[offset + i * m_dimension + j];
                }
                tokenCount++;
            }
        }

        // Average
        if (tokenCount > 0)
        {
            for (auto& val : embedding)
            {
                val /= tokenCount;
            }
        }

        // Normalize to unit vector
        normalizeVector(embedding);

        return embedding;
    }

    std::string resolveModelPath(const std::string& path)
    {
        if (std::filesystem::exists(path))
        {
            return path;
        }

        // Try ~/.fmf/models/
        std::filesystem::path homePath =
            std::filesystem::path(std::getenv("HOME"));
        std::filesystem::path modelDir = homePath / ".fmf" / "models";
        std::filesystem::path fullPath = modelDir / path;

        if (std::filesystem::exists(fullPath))
        {
            return fullPath.string();
        }

        // Return original path (will fail in initializeModel)
        return path;
    }

    void normalizeVector(std::vector<float>& vec)
    {
        float norm = 0.0f;
        for (float val : vec)
        {
            norm += val * val;
        }
        norm = std::sqrt(norm);

        if (norm > 1e-8f)
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
    SimpleTokenizer m_tokenizer;

    // ONNX Runtime objects
    Ort::Env m_env;
    Ort::SessionOptions m_sessionOptions;
    std::unique_ptr<Ort::Session> m_session;
    std::vector<std::string> m_inputNames;
    std::vector<std::string> m_outputNames;
};

#else  // !ENABLE_ONNX_RUNTIME

/**
 * @brief Placeholder implementation (when ONNX Runtime not available)
 */
class LocalEmbeddingProvider::Impl
{
 public:
    explicit Impl(const std::string& modelPath, bool useGpu)
        : m_modelPath(modelPath),
          m_useGpu(useGpu),
          m_dimension(384),
          m_tokenizer(SimpleTokenizer::DEFAULT_MAX_LENGTH, true)
    {
        initializeModel();
    }

    std::vector<float> infer(const std::vector<int64_t>& tokens)
    {
        // Placeholder: return deterministic "fake" embedding based on tokens
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

        normalizeVector(embedding);
        return embedding;
    }

    std::vector<std::vector<float>> inferBatch(
        const std::vector<std::vector<int64_t>>& batchTokens)
    {
        std::vector<std::vector<float>> embeddings;
        embeddings.reserve(batchTokens.size());

        for (const auto& tokens : batchTokens)
        {
            embeddings.push_back(infer(tokens));
        }

        return embeddings;
    }

    size_t getDimension() const { return m_dimension; }

    std::vector<int64_t> tokenize(const std::string& text) const
    {
        return m_tokenizer.tokenize(text);
    }

 private:
    void initializeModel()
    {
        if (!std::filesystem::exists(m_modelPath))
        {
            throw std::runtime_error("Model file not found: " + m_modelPath);
        }
    }

    void normalizeVector(std::vector<float>& vec)
    {
        float norm = 0.0f;
        for (float val : vec)
        {
            norm += val * val;
        }
        norm = std::sqrt(norm);

        if (norm > 1e-8f)
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
    SimpleTokenizer m_tokenizer;
};

#endif  // ENABLE_ONNX_RUNTIME

// Public API implementation (same for both versions)

// Public API implementation (same for both versions)

LocalEmbeddingProvider::LocalEmbeddingProvider(const std::string& modelPath,
                                               bool useGpu)
    : m_impl(std::make_unique<Impl>(modelPath, useGpu))
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
    return m_impl->tokenize(text);
}

}  // namespace fmf
