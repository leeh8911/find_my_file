/**
 * @file semantic_result.h
 * @brief Semantic search result structure
 *
 * Represents a file found by semantic search with relevance score
 * and matched text chunks.
 */

#ifndef FMF_SEMANTIC_RESULT_H
#define FMF_SEMANTIC_RESULT_H

#pragma once

#include <string>
#include <vector>

#include "domain/entities/file_info.h"

namespace fmf
{

/**
 * @brief Result from semantic search
 *
 * Contains file information, relevance score, and matched text chunks.
 */
struct SemanticResult
{
    FileInfo fileInfo;                       ///< File metadata
    float relevanceScore;                    ///< Similarity score (0.0 ~ 1.0)
    std::vector<std::string> matchedChunks;  ///< Text chunks that matched

    /**
     * @brief Constructor for empty path result
     */
    SemanticResult() : fileInfo(std::filesystem::path("")), relevanceScore(0.0f)
    {
    }

    /**
     * @brief Constructor with file info and score
     */
    SemanticResult(const FileInfo& info, float score)
        : fileInfo(info), relevanceScore(score)
    {
    }

    /**
     * @brief Constructor with all fields
     */
    SemanticResult(const FileInfo& info, float score,
                   const std::vector<std::string>& chunks)
        : fileInfo(info), relevanceScore(score), matchedChunks(chunks)
    {
    }

    /**
     * @brief Compare by relevance score (for sorting)
     */
    bool operator>(const SemanticResult& other) const
    {
        return relevanceScore > other.relevanceScore;
    }
};

}  // namespace fmf

#endif  // FMF_SEMANTIC_RESULT_H
