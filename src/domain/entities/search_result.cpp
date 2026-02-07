#include "domain/entities/search_result.h"

#include <algorithm>

namespace fmf
{

void SearchResult::addFile(const FileInfo& fileInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    files_.push_back(fileInfo);
}

void SearchResult::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    files_.clear();
}

void SearchResult::sortByName(bool ascending)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::sort(files_.begin(), files_.end(),
              [ascending](const FileInfo& a, const FileInfo& b)
              {
                  if (ascending)
                  {
                      return a.getFileName() < b.getFileName();
                  }
                  else
                  {
                      return a.getFileName() > b.getFileName();
                  }
              });
}

void SearchResult::sortBySize(bool ascending)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::sort(files_.begin(), files_.end(),
              [ascending](const FileInfo& a, const FileInfo& b)
              {
                  if (ascending)
                  {
                      return a.getSize() < b.getSize();
                  }
                  else
                  {
                      return a.getSize() > b.getSize();
                  }
              });
}

void SearchResult::sortByTime(bool ascending)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::sort(files_.begin(), files_.end(),
              [ascending](const FileInfo& a, const FileInfo& b)
              {
                  if (ascending)
                  {
                      return a.getLastWriteTime() < b.getLastWriteTime();
                  }
                  else
                  {
                      return a.getLastWriteTime() > b.getLastWriteTime();
                  }
              });
}

}  // namespace fmf
