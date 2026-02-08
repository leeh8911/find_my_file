#ifndef FMF_IMAGE_CAPTION_SERVICE_H
#define FMF_IMAGE_CAPTION_SERVICE_H

#pragma once

#include <string>

namespace fmf
{

struct CaptionResult
{
    std::string text;
    std::string modelName;
};

class ImageCaptionService
{
 public:
    explicit ImageCaptionService(const std::string& modelName);

    CaptionResult generateCaption(const std::string& imagePath) const;
    std::string getModelName() const;

 private:
    std::string runExternalCaption(const std::string& imagePath) const;

    std::string modelName_;
    std::string captionCommand_;
};

}  // namespace fmf

#endif  // FMF_IMAGE_CAPTION_SERVICE_H
