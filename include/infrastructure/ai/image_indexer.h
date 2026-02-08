#ifndef FMF_IMAGE_INDEXER_H
#define FMF_IMAGE_INDEXER_H

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "infrastructure/ai/image_caption_service.h"
#include "infrastructure/ai/image_index_text_builder.h"
#include "infrastructure/ai/image_ocr_service.h"
#include "infrastructure/ai/local_embedding_provider.h"

namespace fmf
{

struct IndexedImage
{
    std::string filePath;
    std::string fileHash;
    std::string createdAt;
    std::string updatedAt;
    std::string ocrText;
    std::string captionText;
    std::string indexText;
    std::optional<double> ocrConfidence;
    std::string ocrEngineVersion;
    std::string captionModelName;
    std::string embeddingModelName;
    size_t embeddingDim = 0;
    std::string embeddingVersionId;
};

class ImageIndexer
{
 public:
    ImageIndexer(const std::string& indexDbPath,
                 const std::vector<std::string>& ocrLanguages,
                 const std::string& captionModelName,
                 const std::string& embeddingModelPath);
    ~ImageIndexer();

    std::vector<IndexedImage> indexPath(const std::string& path);
    std::vector<std::pair<std::string, double>> search(const std::string& query,
                                                       size_t topK) const;

    std::string getEmbeddingVersionId() const;

 private:
    void ensureSchema();
    void ensureMeta();
    void markStaleIfVersionChanged(const std::string& previousVersion);
    void upsert(const IndexedImage& record,
                const std::vector<float>& embedding);
    std::string buildEmbeddingVersionId() const;
    std::string hashFile(const std::string& path) const;
    std::pair<std::string, std::string> fileTimes(
        const std::string& path) const;

    IndexedImage indexImage(const std::string& path);

    std::string indexDbPath_;
    std::vector<std::string> ocrLanguages_;
    ImageIndexTextBuilder textBuilder_;
    ImageOcrService ocrService_;
    ImageCaptionService captionService_;
    LocalEmbeddingProvider embedder_;
    std::string embeddingModelName_;
    std::string embeddingVersionId_;

    void* dbHandle_ = nullptr;
};

}  // namespace fmf

#endif  // FMF_IMAGE_INDEXER_H
