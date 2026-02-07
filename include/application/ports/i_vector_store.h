/**
 * @file i_vector_store.h
 * @brief Interface for vector storage and similarity search
 *
 * Defines abstract interface for storing and searching vector embeddings.
 * Implementations can use FAISS, ChromaDB, or in-memory storage.
 *
 * SOLID Principles:
 * - ISP: Focused interface for vector operations
 * - DIP: SemanticSearcher depends on this abstraction
 * - OCP: New stores can be added without modifying existing code
 */

#ifndef FMF_I_VECTOR_STORE_H
#define FMF_I_VECTOR_STORE_H

#pragma once

#include <string>
#include <vector>

#include "domain/entities/semantic_result.h"

namespace fmf
{

/**
 * @brief Metadata associated with a stored vector
 */
struct VectorMetadata
{
    std::string filepath;    ///< File path
    std::string chunkText;   ///< Original text chunk
    size_t chunkIndex;       ///< Index of chunk in file
    uint64_t fileTimestamp;  ///< File modification time (for update detection)

    VectorMetadata() : chunkIndex(0), fileTimestamp(0) {}
};

/**
 * @brief Abstract interface for vector stores
 *
 * Provides storage, similarity search, and persistence for vector embeddings.
 *
 * Example implementations:
 * - FAISSStore: Uses FAISS library for fast approximate search
 * - ChromaDBStore: Uses ChromaDB for persistent vector storage
 * - MockStore: In-memory map for testing
 */
class IVectorStore
{
 public:
    virtual ~IVectorStore() = default;

    /**
     * @brief Add a vector with metadata
     *
     * @param id Unique identifier for this vector
     * @param vector Embedding vector
     * @param metadata Associated metadata
     *
     * @throws std::runtime_error on storage errors
     */
    virtual void add(const std::string& id, const std::vector<float>& vector,
                     const VectorMetadata& metadata) = 0;

    /**
     * @brief Search for similar vectors
     *
     * Uses cosine similarity or other distance metrics.
     *
     * @param queryVector Query embedding
     * @param topK Number of results to return
     * @return Top-K most similar results with scores
     *
     * @throws std::runtime_error on search errors
     */
    virtual std::vector<SemanticResult> search(
        const std::vector<float>& queryVector, size_t topK) = 0;

    /**
     * @brief Remove a vector by ID
     *
     * @param id Vector identifier
     * @return true if removed, false if not found
     */
    virtual bool remove(const std::string& id) = 0;

    /**
     * @brief Save index to disk
     *
     * @param path Directory path to save index
     * @return true on success
     *
     * @throws std::runtime_error on I/O errors
     */
    virtual bool save(const std::string& path) = 0;

    /**
     * @brief Load index from disk
     *
     * @param path Directory path containing saved index
     * @return true on success, false if not found
     *
     * @throws std::runtime_error on I/O errors or corrupt data
     */
    virtual bool load(const std::string& path) = 0;

    /**
     * @brief Get number of vectors in store
     *
     * @return Count of stored vectors
     */
    virtual size_t size() const = 0;

    /**
     * @brief Clear all vectors
     */
    virtual void clear() = 0;
};

}  // namespace fmf

#endif  // FMF_I_VECTOR_STORE_H
