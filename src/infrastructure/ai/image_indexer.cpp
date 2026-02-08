#include "infrastructure/ai/image_indexer.h"

#include <sqlite3.h>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

#ifdef FMF_HAVE_OPENSSL
#include <openssl/sha.h>
#endif

#include "infrastructure/ai/image_log.h"

namespace fmf
{

namespace
{
std::string joinLanguages(const std::vector<std::string>& langs)
{
    std::ostringstream out;
    for (size_t i = 0; i < langs.size(); ++i)
    {
        if (i > 0)
        {
            out << "+";
        }
        out << langs[i];
    }
    return out.str();
}

std::string toString(double value)
{
    std::ostringstream out;
    out << value;
    return out.str();
}

std::string utcNow()
{
    std::time_t now = std::time(nullptr);
    std::tm tm = *std::gmtime(&now);
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}
}  // namespace

ImageIndexer::ImageIndexer(const std::string& indexDbPath,
                           const std::vector<std::string>& ocrLanguages,
                           const std::string& captionModelName,
                           const std::string& embeddingModelPath)
    : indexDbPath_(indexDbPath),
      ocrLanguages_(ocrLanguages),
      ocrService_(ocrLanguages),
      captionService_(captionModelName),
      embedder_(embeddingModelPath),
      embeddingModelName_(embeddingModelPath)
{
    std::filesystem::path dbPath(indexDbPath_);
    if (dbPath.has_parent_path())
    {
        std::filesystem::create_directories(dbPath.parent_path());
    }

    if (sqlite3_open(indexDbPath_.c_str(),
                     reinterpret_cast<sqlite3**>(&dbHandle_)) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to open SQLite database");
    }

    if (std::find(ocrLanguages_.begin(), ocrLanguages_.end(), "eng") ==
        ocrLanguages_.end())
    {
        ocrLanguages_.push_back("eng");
    }

    embeddingVersionId_ = buildEmbeddingVersionId();

    ensureSchema();
    ensureMeta();
}

ImageIndexer::~ImageIndexer()
{
    if (dbHandle_)
    {
        sqlite3_close(reinterpret_cast<sqlite3*>(dbHandle_));
    }
}

std::vector<IndexedImage> ImageIndexer::indexPath(const std::string& path)
{
    std::filesystem::path target(path);
    if (!std::filesystem::exists(target))
    {
        throw std::runtime_error("Image path not found");
    }

    std::vector<IndexedImage> results;
    if (std::filesystem::is_regular_file(target))
    {
        results.push_back(indexImage(target.string()));
        return results;
    }

    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(target))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" ||
            ext == ".tiff")
        {
            results.push_back(indexImage(entry.path().string()));
        }
    }

    return results;
}

std::vector<std::pair<std::string, double>> ImageIndexer::search(
    const std::string& query, size_t topK) const
{
    std::vector<std::pair<std::string, double>> results;
    if (!dbHandle_)
    {
        return results;
    }

    auto queryEmbedding = embedder_.generateEmbedding(query);
    double norm = 0.0;
    for (float value : queryEmbedding)
    {
        norm += value * value;
    }
    norm = std::sqrt(norm);
    if (norm > 0.0)
    {
        for (float& value : queryEmbedding)
        {
            value = static_cast<float>(value / norm);
        }
    }

    const char* sql =
        "SELECT file_path, embedding FROM image_embeddings "
        "WHERE embedding_version_id = ? AND stale = 0";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(dbHandle_), sql, -1, &stmt,
                       nullptr);
    sqlite3_bind_text(stmt, 1, embeddingVersionId_.c_str(), -1,
                      SQLITE_TRANSIENT);

    std::vector<std::string> paths;
    std::vector<std::vector<float>> embeddings;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* pathText = sqlite3_column_text(stmt, 0);
        const void* blob = sqlite3_column_blob(stmt, 1);
        int bytes = sqlite3_column_bytes(stmt, 1);
        if (!pathText || !blob || bytes <= 0)
        {
            continue;
        }

        std::string pathStr(reinterpret_cast<const char*>(pathText));
        size_t count = static_cast<size_t>(bytes / sizeof(float));
        const float* data = static_cast<const float*>(blob);
        embeddings.emplace_back(data, data + count);
        paths.push_back(pathStr);
    }

    sqlite3_finalize(stmt);

    for (size_t i = 0; i < embeddings.size(); ++i)
    {
        double score = 0.0;
        for (size_t j = 0;
             j < embeddings[i].size() && j < queryEmbedding.size(); ++j)
        {
            score += embeddings[i][j] * queryEmbedding[j];
        }
        results.emplace_back(paths[i], score);
    }

    std::sort(results.begin(), results.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (results.size() > topK)
    {
        results.resize(topK);
    }

    return results;
}

std::string ImageIndexer::getEmbeddingVersionId() const
{
    return embeddingVersionId_;
}

void ImageIndexer::ensureSchema()
{
    const char* createTable =
        "CREATE TABLE IF NOT EXISTS image_embeddings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "file_path TEXT NOT NULL,"
        "file_hash TEXT NOT NULL,"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL,"
        "ocr_text TEXT,"
        "caption_text TEXT,"
        "index_text TEXT,"
        "ocr_confidence REAL,"
        "ocr_engine_version TEXT,"
        "caption_model_name TEXT,"
        "embedding_model_name TEXT,"
        "embedding_dim INTEGER,"
        "embedding_version_id TEXT,"
        "embedding BLOB NOT NULL,"
        "stale INTEGER DEFAULT 0,"
        "UNIQUE(file_hash, embedding_version_id)"
        ");";

    const char* createMeta =
        "CREATE TABLE IF NOT EXISTS index_meta ("
        "key TEXT PRIMARY KEY,"
        "value TEXT NOT NULL"
        ");";

    char* err = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(dbHandle_), createTable, nullptr,
                 nullptr, &err);
    sqlite3_exec(reinterpret_cast<sqlite3*>(dbHandle_), createMeta, nullptr,
                 nullptr, &err);
    if (err)
    {
        std::string message = err;
        sqlite3_free(err);
        throw std::runtime_error("SQLite schema error: " + message);
    }
}

void ImageIndexer::ensureMeta()
{
    const char* selectMeta = "SELECT key, value FROM index_meta";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(dbHandle_), selectMeta, -1,
                       &stmt, nullptr);

    std::map<std::string, std::string> meta;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* key = sqlite3_column_text(stmt, 0);
        const unsigned char* value = sqlite3_column_text(stmt, 1);
        if (key && value)
        {
            meta[reinterpret_cast<const char*>(key)] =
                reinterpret_cast<const char*>(value);
        }
    }
    sqlite3_finalize(stmt);

    auto it = meta.find("embedding_version_id");
    if (it != meta.end() && it->second != embeddingVersionId_)
    {
        markStaleIfVersionChanged(it->second);
    }

    const char* upsert =
        "INSERT OR REPLACE INTO index_meta (key, value) VALUES (?, ?)";
    sqlite3_stmt* upsertStmt = nullptr;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(dbHandle_), upsert, -1,
                       &upsertStmt, nullptr);

    auto setMeta = [&](const std::string& key, const std::string& value)
    {
        sqlite3_bind_text(upsertStmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(upsertStmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(upsertStmt);
        sqlite3_reset(upsertStmt);
    };

    setMeta("embedding_version_id", embeddingVersionId_);
    setMeta("embedding_model_name", embeddingModelName_);
    setMeta("caption_model_name", captionService_.getModelName());
    setMeta("ocr_langs", joinLanguages(ocrService_.getUsedLanguages()));
    setMeta("merge_rule_version", ImageIndexTextBuilder::MERGE_RULE_VERSION);
    setMeta("updated_at", utcNow());
    if (meta.find("created_at") == meta.end())
    {
        setMeta("created_at", utcNow());
    }

    sqlite3_finalize(upsertStmt);
}

void ImageIndexer::markStaleIfVersionChanged(const std::string& previousVersion)
{
    logImageEvent("index_version_mismatch", "warn",
                  {{"old_version", previousVersion},
                   {"new_version", embeddingVersionId_}});

    const char* sql = "UPDATE image_embeddings SET stale = 1";
    sqlite3_exec(reinterpret_cast<sqlite3*>(dbHandle_), sql, nullptr, nullptr,
                 nullptr);
}

void ImageIndexer::upsert(const IndexedImage& record,
                          const std::vector<float>& embedding)
{
    const char* sql =
        "INSERT OR REPLACE INTO image_embeddings ("
        "file_path, file_hash, created_at, updated_at, ocr_text, caption_text, "
        "index_text, ocr_confidence, ocr_engine_version, caption_model_name, "
        "embedding_model_name, embedding_dim, embedding_version_id, embedding, "
        "stale"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0)";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(dbHandle_), sql, -1, &stmt,
                       nullptr);
    sqlite3_bind_text(stmt, 1, record.filePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, record.fileHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, record.createdAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, record.updatedAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, record.ocrText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, record.captionText.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, record.indexText.c_str(), -1, SQLITE_TRANSIENT);
    if (record.ocrConfidence.has_value())
    {
        sqlite3_bind_double(stmt, 8, record.ocrConfidence.value());
    }
    else
    {
        sqlite3_bind_null(stmt, 8);
    }
    sqlite3_bind_text(stmt, 9, record.ocrEngineVersion.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, record.captionModelName.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, record.embeddingModelName.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 12, static_cast<int>(record.embeddingDim));
    sqlite3_bind_text(stmt, 13, record.embeddingVersionId.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 14, embedding.data(),
                      static_cast<int>(embedding.size() * sizeof(float)),
                      SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::string ImageIndexer::buildEmbeddingVersionId() const
{
    std::string raw = embeddingModelName_ + "|" +
                      captionService_.getModelName() + "|" +
                      joinLanguages(ocrService_.getUsedLanguages()) + "|" +
                      ImageIndexTextBuilder::MERGE_RULE_VERSION;

    std::hash<std::string> hasher;
    std::ostringstream out;
    out << std::hex << hasher(raw);
    return out.str();
}

std::string ImageIndexer::hashFile(const std::string& path) const
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        return "";
    }

#ifdef FMF_HAVE_OPENSSL
    unsigned char hash[32];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    std::vector<unsigned char> buffer(1024 * 1024);
    while (input)
    {
        input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize count = input.gcount();
        if (count > 0)
        {
            SHA256_Update(&ctx, buffer.data(), static_cast<size_t>(count));
        }
    }
    SHA256_Final(hash, &ctx);

    std::ostringstream out;
    for (unsigned char byte : hash)
    {
        out << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte);
    }
    return out.str();
#else
    std::vector<unsigned char> buffer(1024 * 1024);
    std::hash<std::string> hasher;
    std::size_t seed = 0;
    while (input)
    {
        input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize count = input.gcount();
        if (count > 0)
        {
            std::string chunk(reinterpret_cast<char*>(buffer.data()),
                              static_cast<size_t>(count));
            seed ^= hasher(chunk) + 0x9e3779b97f4a7c15ULL + (seed << 6) +
                    (seed >> 2);
        }
    }

    std::ostringstream out;
    out << std::hex << seed;
    return out.str();
#endif
}

std::pair<std::string, std::string> ImageIndexer::fileTimes(
    const std::string& path) const
{
    auto stat = std::filesystem::last_write_time(path);
    auto time = decltype(stat)::clock::to_time_t(stat);
    std::tm tm = *std::gmtime(&time);

    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return {out.str(), out.str()};
}

IndexedImage ImageIndexer::indexImage(const std::string& path)
{
    auto times = fileTimes(path);
    std::string fileHash = hashFile(path);

    logImageEvent("image_index_start", "info", {{"file_path", path}});

    auto ocrResult = ocrService_.extractText(path);
    if (joinLanguages(ocrLanguages_) !=
        joinLanguages(ocrService_.getUsedLanguages()))
    {
        logImageEvent(
            "ocr_language_fallback", "warn",
            {{"requested", joinLanguages(ocrLanguages_)},
             {"used", joinLanguages(ocrService_.getUsedLanguages())}});
    }

    logImageEvent(
        "ocr_complete", "info",
        {{"file_path", path},
         {"ocr_languages", ocrResult.languages},
         {"ocr_confidence", ocrResult.confidence.has_value()
                                ? toString(ocrResult.confidence.value())
                                : ""}});

    auto captionResult = captionService_.generateCaption(path);
    logImageEvent(
        "caption_complete", "info",
        {{"file_path", path}, {"caption_model", captionResult.modelName}});

    auto indexTextResult =
        textBuilder_.build(ocrResult.text, captionResult.text);
    logImageEvent("index_text_built", "info",
                  {{"file_path", path},
                   {"index_text_length",
                    std::to_string(indexTextResult.indexText.size())}});

    auto embedding = embedder_.generateEmbedding(indexTextResult.indexText);
    size_t embeddingDim = embedding.size();
    logImageEvent("embedding_generated", "info",
                  {{"file_path", path},
                   {"embedding_dim", std::to_string(embeddingDim)},
                   {"embedding_model", embeddingModelName_}});

    IndexedImage record;
    record.filePath = path;
    record.fileHash = fileHash;
    record.createdAt = times.first;
    record.updatedAt = times.second;
    record.ocrText = indexTextResult.ocrText;
    record.captionText = indexTextResult.captionText;
    record.indexText = indexTextResult.indexText;
    record.ocrConfidence = ocrResult.confidence;
    record.ocrEngineVersion = ocrResult.engineVersion;
    record.captionModelName = captionResult.modelName;
    record.embeddingModelName = embeddingModelName_;
    record.embeddingDim = embeddingDim;
    record.embeddingVersionId = embeddingVersionId_;

    upsert(record, embedding);
    logImageEvent("image_indexed", "info", {{"file_path", path}});
    return record;
}

}  // namespace fmf
