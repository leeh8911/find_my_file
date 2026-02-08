#ifndef FMF_IMAGE_OCR_SERVICE_H
#define FMF_IMAGE_OCR_SERVICE_H

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace fmf
{

struct OcrResult
{
    std::string text;
    std::optional<double> confidence;
    std::string languages;
    std::string engineVersion;
};

class ImageOcrService
{
 public:
    explicit ImageOcrService(const std::vector<std::string>& languages);

    OcrResult extractText(const std::string& imagePath);
    std::vector<std::string> getRequestedLanguages() const;
    std::vector<std::string> getUsedLanguages() const;
    std::string getEngineVersion() const;

 private:
    std::vector<std::string> selectLanguages() const;
    std::optional<double> extractConfidence(void* pix) const;

    std::vector<std::string> requestedLanguages_;
    std::vector<std::string> availableLanguages_;
    std::vector<std::string> usedLanguages_;
    std::string engineVersion_;
};

}  // namespace fmf

#endif  // FMF_IMAGE_OCR_SERVICE_H
