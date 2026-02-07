#include "semantic_searcher.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace fmf
{

SemanticSearcher::SemanticSearcher(
    std::unique_ptr<IEmbeddingProvider> provider,
    std::unique_ptr<IVectorStore> store)
    : provider_(std::move(provider)), store_(std::move(store)), nextId_(0)
{
}

bool SemanticSearcher::indexFile(const std::filesystem::path& filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        return false;
    }

    if (!isTextFile(filepath))
    {
        return false;  // Skip binary files
    }

    // Read file content
    std::string content = readFileContent(filepath);
    if (content.empty())
    {
        return false;
    }

    // Chunk the content
    auto chunks = chunkText(content);

    // Generate embeddings for each chunk
    for (size_t i = 0; i < chunks.size(); ++i)
    {
        auto embedding = provider_->generateEmbedding(chunks[i]);

        // Create metadata
        VectorMetadata metadata;
        metadata.filepath = filepath.string();
        metadata.chunkText = chunks[i];
        metadata.chunkIndex = i;
        metadata.fileTimestamp =
            std::filesystem::last_write_time(filepath).time_since_epoch().count();

        // Store in vector store
        std::string id = filepath.string() + "_chunk_" + std::to_string(i);
        store_->add(id, embedding, metadata);
    }

    return true;
}

size_t SemanticSearcher::indexDirectory(const std::filesystem::path& dirpath,
                                        bool recursive)
{
    if (!std::filesystem::exists(dirpath))
    {
        return 0;
    }

    if (!std::filesystem::is_directory(dirpath))
    {
        return 0;
    }

    size_t indexed = 0;

    if (recursive)
    {
        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(dirpath))
        {
            if (entry.is_regular_file())
            {
                if (indexFile(entry.path()))
                {
                    ++indexed;
                }
            }
        }
    }
    else
    {
        for (const auto& entry : std::filesystem::directory_iterator(dirpath))
        {
            if (entry.is_regular_file())
            {
                if (indexFile(entry.path()))
                {
                    ++indexed;
                }
            }
        }
    }

    return indexed;
}

std::vector<SemanticResult> SemanticSearcher::search(const std::string& query,
                                                      size_t topK)
{
    // Generate embedding for query
    auto queryEmbedding = provider_->generateEmbedding(query);

    // Search in vector store
    auto results = store_->search(queryEmbedding, topK);

    return results;
}

std::vector<SemanticResult> SemanticSearcher::searchSimilar(
    const std::filesystem::path& filepath, size_t topK)
{
    if (!std::filesystem::exists(filepath))
    {
        return {};
    }

    // Read file content
    std::string content = readFileContent(filepath);
    if (content.empty())
    {
        return {};
    }

    // Use the content as query
    return search(content, topK);
}

void SemanticSearcher::clear() { store_->clear(); }

size_t SemanticSearcher::size() const { return store_->size(); }

std::vector<std::string> SemanticSearcher::chunkText(const std::string& text,
                                                      size_t chunkSize,
                                                      size_t overlap)
{
    std::vector<std::string> chunks;

    if (text.empty())
    {
        return chunks;
    }

    // If text is smaller than chunk size, return as single chunk
    if (text.length() <= chunkSize)
    {
        chunks.push_back(text);
        return chunks;
    }

    // Split into chunks with overlap
    size_t start = 0;
    while (start < text.length())
    {
        size_t end = std::min(start + chunkSize, text.length());
        chunks.push_back(text.substr(start, end - start));

        // Move to next chunk with overlap
        if (end >= text.length())
        {
            break;
        }

        start += chunkSize - overlap;
    }

    return chunks;
}

std::string SemanticSearcher::readFileContent(
    const std::filesystem::path& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool SemanticSearcher::isTextFile(const std::filesystem::path& filepath)
{
    // Simple heuristic: check file extension
    std::string ext = filepath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    static const std::vector<std::string> textExtensions = {
        ".txt",  ".md",   ".cpp",  ".c",   ".h",    ".hpp", ".py",
        ".js",   ".java", ".cs",   ".go",  ".rs",   ".rb",  ".php",
        ".html", ".css",  ".json", ".xml", ".yaml", ".yml", ".sh"};

    return std::find(textExtensions.begin(), textExtensions.end(), ext) !=
           textExtensions.end();
}

}  // namespace fmf
