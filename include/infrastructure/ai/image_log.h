#ifndef FMF_IMAGE_LOG_H
#define FMF_IMAGE_LOG_H

#pragma once

#include <map>
#include <string>

namespace fmf
{

void logImageEvent(const std::string& event, const std::string& level,
                   const std::map<std::string, std::string>& fields);

}  // namespace fmf

#endif  // FMF_IMAGE_LOG_H
