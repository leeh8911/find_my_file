#include "infrastructure/ai/image_caption_service.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <cstdio>
#define FMF_POPEN _popen
#define FMF_PCLOSE _pclose
#else
#include <cstdio>
#define FMF_POPEN popen
#define FMF_PCLOSE pclose
#endif

namespace fmf
{

ImageCaptionService::ImageCaptionService(const std::string& modelName)
    : modelName_(modelName)
{
    const char* cmd = std::getenv("FMF_CAPTION_CMD");
    if (cmd)
    {
        captionCommand_ = cmd;
    }
    else
    {
        captionCommand_ = "blip-caption";
    }
}

CaptionResult ImageCaptionService::generateCaption(
    const std::string& imagePath) const
{
    if (captionCommand_.empty())
    {
        throw std::runtime_error(
            "Caption command not configured. Set FMF_CAPTION_CMD.");
    }

    CaptionResult result;
    result.text = runExternalCaption(imagePath);
    result.modelName = modelName_;
    return result;
}

std::string ImageCaptionService::getModelName() const { return modelName_; }

std::string ImageCaptionService::runExternalCaption(
    const std::string& imagePath) const
{
    std::string command = captionCommand_ + " \"" + imagePath + "\"";
    FILE* pipe = FMF_POPEN(command.c_str(), "r");
    if (!pipe)
    {
        throw std::runtime_error("Failed to run caption command");
    }

    std::string output;
    char buffer[256];
    while (std::fgets(buffer, sizeof(buffer), pipe))
    {
        output += buffer;
    }
    FMF_PCLOSE(pipe);

    while (!output.empty() && (output.back() == '\n' || output.back() == '\r'))
    {
        output.pop_back();
    }

    return output;
}

}  // namespace fmf
