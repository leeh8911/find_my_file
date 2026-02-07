#ifndef SEARCH_RESULT_H
#define SEARCH_RESULT_H

#include <memory>
#include <mutex>
#include <vector>

#include "domain/entities/file_info.h"

namespace fmf
{

/**
 * @brief Container for search results
 */
class SearchResult
{
 public:
    /**
     * @brief Default constructor
     */
    SearchResult() = default;

    /**
     * @brief Move constructor
     */
    SearchResult(SearchResult&& other) noexcept
        : files_(std::move(other.files_))
    {
    }

    /**
     * @brief Move assignment operator
     */
    SearchResult& operator=(SearchResult&& other) noexcept
    {
        if (this != &other)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            files_ = std::move(other.files_);
        }
        return *this;
    }

    // Delete copy constructor and assignment
    SearchResult(const SearchResult&) = delete;
    SearchResult& operator=(const SearchResult&) = delete;

    /**
     * @brief Add a file to the results
     * @param fileInfo FileInfo to add
     */
    void addFile(const FileInfo& fileInfo);

    /**
     * @brief Get all files in the results
     * @return Vector of FileInfo objects
     */
    const std::vector<FileInfo>& getFiles() const { return files_; }

    /**
     * @brief Get the number of files in the results
     * @return Number of files
     */
    size_t size() const { return files_.size(); }

    /**
     * @brief Check if results are empty
     * @return true if empty, false otherwise
     */
    bool empty() const { return files_.empty(); }

    /**
     * @brief Clear all results
     */
    void clear();

    /**
     * @brief Sort results by file name
     * @param ascending Sort in ascending order if true, descending if false
     */
    void sortByName(bool ascending = true);

    /**
     * @brief Sort results by file size
     * @param ascending Sort in ascending order if true, descending if false
     */
    void sortBySize(bool ascending = true);

    /**
     * @brief Sort results by last write time
     * @param ascending Sort in ascending order if true, descending if false
     */
    void sortByTime(bool ascending = true);

    /**
     * @brief Get iterator to beginning
     */
    std::vector<FileInfo>::iterator begin() { return files_.begin(); }
    std::vector<FileInfo>::const_iterator begin() const
    {
        return files_.begin();
    }

    /**
     * @brief Get iterator to end
     */
    std::vector<FileInfo>::iterator end() { return files_.end(); }
    std::vector<FileInfo>::const_iterator end() const { return files_.end(); }

 private:
    std::vector<FileInfo> files_;
    mutable std::mutex mutex_;  // For thread-safe operations
};

}  // namespace fmf

#endif  // SEARCH_RESULT_H
