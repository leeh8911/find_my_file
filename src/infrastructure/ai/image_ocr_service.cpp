#include "infrastructure/ai/image_ocr_service.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

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
}  // namespace

ImageOcrService::ImageOcrService(const std::vector<std::string>& languages)
    : requestedLanguages_(languages.empty()
                              ? std::vector<std::string>{"kor", "eng"}
                              : languages)
{
    tesseract::TessBaseAPI api;
    std::vector<std::string> available;
    api.GetAvailableLanguagesAsVector(&available);
    availableLanguages_ = available;
    usedLanguages_ = selectLanguages();

    api.Init(nullptr, joinLanguages(usedLanguages_).c_str());
    engineVersion_ = api.Version();
    api.End();
}

OcrResult ImageOcrService::extractText(const std::string& imagePath)
{
    std::string lang = joinLanguages(usedLanguages_);
    tesseract::TessBaseAPI api;
    if (api.Init(nullptr, lang.c_str()) != 0)
    {
        throw std::runtime_error("Failed to initialize Tesseract");
    }

    Pix* pix = pixRead(imagePath.c_str());
    if (!pix)
    {
        api.End();
        throw std::runtime_error("Failed to load image for OCR");
    }

    api.SetImage(pix);
    char* text = api.GetUTF8Text();
    std::string resultText = text ? text : "";
    delete[] text;

    std::optional<double> confidence = extractConfidence(pix);

    api.End();
    pixDestroy(&pix);

    OcrResult result;
    result.text = resultText;
    result.confidence = confidence;
    result.languages = lang;
    result.engineVersion = engineVersion_;
    return result;
}

std::vector<std::string> ImageOcrService::getRequestedLanguages() const
{
    return requestedLanguages_;
}

std::vector<std::string> ImageOcrService::getUsedLanguages() const
{
    return usedLanguages_;
}

std::string ImageOcrService::getEngineVersion() const { return engineVersion_; }

std::vector<std::string> ImageOcrService::selectLanguages() const
{
    if (availableLanguages_.empty())
    {
        return {"eng"};
    }

    std::vector<std::string> selected;
    for (const auto& lang : requestedLanguages_)
    {
        if (std::find(availableLanguages_.begin(), availableLanguages_.end(),
                      lang) != availableLanguages_.end())
        {
            selected.push_back(lang);
        }
    }

    if (selected.empty())
    {
        selected.push_back("eng");
    }

    return selected;
}

std::optional<double> ImageOcrService::extractConfidence(void* pix) const
{
    tesseract::TessBaseAPI api;
    std::string lang = joinLanguages(usedLanguages_);
    if (api.Init(nullptr, lang.c_str()) != 0)
    {
        api.End();
        return std::nullopt;
    }

    api.SetImage(static_cast<Pix*>(pix));
    int* confidences = api.AllWordConfidences();
    if (!confidences)
    {
        api.End();
        return std::nullopt;
    }

    double sum = 0.0;
    int count = 0;
    for (int i = 0; confidences[i] != -1; ++i)
    {
        sum += confidences[i];
        count++;
    }
    delete[] confidences;
    api.End();

    if (count == 0)
    {
        return std::nullopt;
    }
    return sum / count;
}

}  // namespace fmf
