#ifndef MOCK_VECTOR_STORE_H
#define MOCK_VECTOR_STORE_H

#include "file_info.h"
#include "i_vector_store.h"
#include "semantic_result.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace fmf
{

/**
 * @brief Mock implementation of IVectorStore for testing
 *
 * This class provides an in-memory vector store with basic
 * cosine similarity search. It's designed for unit testing
 * and does not require external dependencies like FAISS.
 *
 * Features:
 * - In-memory storage (std::map based)
 * - Cosine similarity search
 * - Simple persistence (mock only)
 * - Thread-safe for single-threaded tests
 *
 * @note This is NOT optimized for large-scale production use.
 *       Use FAISSStore or ChromaDBStore for real applications.
 *
 * Example usage:
 * @code
 *   MockVectorStore store;
 *   store.add("id1", embedding, metadata);
 *   auto results = store.search(queryVector, 5);
 * @endcode
 */
class MockVectorStore : public IVectorStore
{
 public:
    /**
     * @brief Add a vector with metadata to the store
     *
     * @param id Unique identifier for this vector
     * @param vector The embedding vector
     * @param metadata Metadata about the file/chunk
     */
    void add(const std::string& id, const std::vector<float>& vector,
             const VectorMetadata& metadata) override
    {
        vectors_[id] = vector;
        metadata_[id] = metadata;
    }

    /**
     * @brief Search for similar vectors using cosine similarity
     *
     * @param queryVector The query embedding vector
     * @param topK Number of top results to return
     * @return Vector of SemanticResult sorted by relevance
     */
    std::vector<SemanticResult> search(const std::vector<float>& queryVector,
                                       size_t topK) override
    {
        std::vector<SemanticResult> results;

        // Compute similarity for each stored vector
        for (const auto& [id, vector] : vectors_)
        {
            float similarity = computeSimilarity(queryVector, vector);

            FileInfo fileInfo(metadata_[id].filepath);
            SemanticResult result(fileInfo, similarity);
            result.matchedChunks.push_back(metadata_[id].chunkText);

            results.push_back(result);
        }

        // Sort by relevance score (descending)
        std::sort(results.begin(), results.end(),
                  [](const SemanticResult& a, const SemanticResult& b) {
                      return a.relevanceScore > b.relevanceScore;
                  });

        // Return top-K results
        if (results.size() > topK)
        {
            results.resize(topK);
        }

        return results;
    }

    /**
     * @brief Remove a vector by ID
     *
     * @param id The identifier to remove
     * @return true if removed, false if not found
     */
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

    /**
     * @brief Save the index to disk (mock implementation)
     *
     * @param path Path to save the index
     * @return true (always succeeds in mock)
     */
    bool save(const std::string& path) override
    {
        (void)path;  // Mark as unused
        // Mock: always succeed
        return true;
    }

    /**
     * @brief Load the index from disk (mock implementation)
     *
     * @param path Path to load the index from
     * @return true (always succeeds in mock)
     */
    bool load(const std::string& path) override
    {
        (void)path;  // Mark as unused
        // Mock: always succeed
        return true;
    }

    /**
     * @brief Get the number of vectors in the store
     * @return Number of stored vectors
     */
    size_t size() const override { return vectors_.size(); }

    /**
     * @brief Clear all vectors from the store
     */
    void clear() override
    {
        vectors_.clear();
        metadata_.clear();
    }

 private:
    /**
     * @brief Compute cosine similarity between two vectors
     *
     * @param a First vector
     * @param b Second vector
     * @return Similarity score in range [0, 1]
     */
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

    std::map<std::string, std::vector<float>> vectors_;      ///< Stored vectors
    std::map<std::string, VectorMetadata> metadata_;  ///< Vector metadata
};

}  // namespace fmf

#endif  // MOCK_VECTOR_STORE_H
