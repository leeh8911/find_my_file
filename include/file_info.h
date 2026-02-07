#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <chrono>
#include <filesystem>
#include <string>

namespace fmf
{

/**
 * @brief File type enumeration
 */
enum class FileType
{
    Regular,
    Directory,
    SymbolicLink,
    Unknown
};

/**
 * @brief Represents information about a file or directory
 */
class FileInfo
{
 public:
    /**
     * @brief Construct from filesystem path
     * @param path The filesystem path
     */
    explicit FileInfo(const std::filesystem::path& path);

    /**
     * @brief Get the full path
     * @return Full path as string
     */
    std::string getPath() const { return path_.string(); }

    /**
     * @brief Get the file name
     * @return File name as string
     */
    std::string getFileName() const { return path_.filename().string(); }

    /**
     * @brief Get the file extension
     * @return File extension as string (including the dot)
     */
    std::string getExtension() const { return path_.extension().string(); }

    /**
     * @brief Get the file size in bytes
     * @return File size
     */
    std::uintmax_t getSize() const { return size_; }

    /**
     * @brief Get the file type
     * @return FileType enum value
     */
    FileType getType() const { return type_; }

    /**
     * @brief Get the last write time
     * @return Last write time as file_time_type
     */
    std::filesystem::file_time_type getLastWriteTime() const
    {
        return lastWriteTime_;
    }

    /**
     * @brief Check if file is a directory
     * @return true if directory, false otherwise
     */
    bool isDirectory() const { return type_ == FileType::Directory; }

    /**
     * @brief Check if file is a regular file
     * @return true if regular file, false otherwise
     */
    bool isRegularFile() const { return type_ == FileType::Regular; }

    /**
     * @brief Check if file is a symbolic link
     * @return true if symbolic link, false otherwise
     */
    bool isSymbolicLink() const { return type_ == FileType::SymbolicLink; }

 private:
    std::filesystem::path path_;
    std::uintmax_t size_;
    FileType type_;
    std::filesystem::file_time_type lastWriteTime_;
};

}  // namespace fmf

#endif  // FILE_INFO_H
