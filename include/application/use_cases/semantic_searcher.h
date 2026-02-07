#ifndef SEMANTIC_SEARCHER_H
#define SEMANTIC_SEARCHER_H

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "domain/entities/semantic_result.h"
#include "application/ports/i_embedding_provider.h"
#include "application/ports/i_vector_store.h"

namespace fmf
{

/**
 * @brief Core semantic search functionality
 *
 * This class provides semantic file search capabilities using
 * embedding models and vector similarity search. It can index
 * files and directories, and perform semantic queries.
 *
 * Features:
 * - Index individual files or entire directories
 * - Search by natural language query
 * - Find similar files
 * - Automatic chunking of large files
 * - Incremental index updates
 *
 * @note This class follows the Dependency Inversion Principle (DIP)
 *       by depending on abstract interfaces (IEmbeddingProvider,
 *       IVectorStore) rather than concrete implementations.
 *
 * Example usage:
 * @code
 *   auto provider = std::make_unique<OpenAIProvider>("api-key");
 *   auto store = std::make_unique<FAISSStore>();
 *   SemanticSearcher searcher(std::move(provider), std::move(store));
 *
 *   searcher.indexDirectory("/path/to/code");
 *   auto results = searcher.search("authentication logic", 5);
 * @endcode
 */
class SemanticSearcher
{
 public:
    /**
     * @brief Construct with provider and store
     *
     * @param provider Embedding provider for text-to-vector conversion
     * @param store Vector store for similarity search
     */
    SemanticSearcher(std::unique_ptr<IEmbeddingProvider> provider,
                     std::unique_ptr<IVectorStore> store);

    /**
     * @brief Index a single file
     *
     * Reads the file content, chunks it, generates embeddings,
     * and stores them in the vector store.
     *
     * @param filepath Path to the file to index
     * @return true if successfully indexed, false otherwise
     */
    bool indexFile(const std::filesystem::path& filepath);

    /**
     * @brief Index all files in a directory (recursive)
     *
     * @param dirpath Path to the directory to index
     * @param recursive Whether to index subdirectories (default: true)
     * @return Number of files successfully indexed
     */
    size_t indexDirectory(const std::filesystem::path& dirpath,
                          bool recursive = true);

    /**
     * @brief Search for files matching a natural language query
     *
     * @param query Natural language query string
     * @param topK Number of top results to return
     * @return Vector of SemanticResult sorted by relevance
     */
    std::vector<SemanticResult> search(const std::string& query, size_t topK);

    /**
     * @brief Find files similar to a given file
     *
     * @param filepath Path to the reference file
     * @param topK Number of similar files to return
     * @return Vector of SemanticResult sorted by similarity
     */
    std::vector<SemanticResult> searchSimilar(
        const std::filesystem::path& filepath, size_t topK);

    /**
     * @brief Clear all indexed data
     */
    void clear();

    /**
     * @brief Get number of indexed chunks
     * @return Number of chunks in the index
     */
    size_t size() const;

 private:
    /**
     * @brief Chunk a large text into smaller pieces
     *
     * Uses fixed-size chunking with overlap for better context.
     *
     * @param text Text to chunk
     * @param chunkSize Size of each chunk in characters
     * @param overlap Overlap between chunks in characters
     * @return Vector of text chunks
     */
    std::vector<std::string> chunkText(const std::string& text,
                                       size_t chunkSize = 1000,
                                       size_t overlap = 200);

    /**
     * @brief Read text content from a file
     *
     * @param filepath Path to the file
     * @return File content as string
     */
    std::string readFileContent(const std::filesystem::path& filepath);

    /**
     * @brief Check if file is a text file
     *
     * @param filepath Path to check
     * @return true if likely a text file
     */
    bool isTextFile(const std::filesystem::path& filepath);

    std::unique_ptr<IEmbeddingProvider> provider_;  ///< Embedding provider
    std::unique_ptr<IVectorStore> store_;           ///< Vector store
    size_t nextId_;  ///< Counter for generating unique IDs
};

}  // namespace fmf

#endif  // SEMANTIC_SEARCHER_H
