#include "domain/entities/file_info.h"

#include <stdexcept>

namespace fmf
{

FileInfo::FileInfo(const std::filesystem::path& path)
    : path_(path), size_(0), type_(FileType::Unknown)
{
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("File or directory does not exist: " +
                                 path.string());
    }

    // Determine file type
    if (std::filesystem::is_symlink(path))
    {
        type_ = FileType::SymbolicLink;
    }
    else if (std::filesystem::is_directory(path))
    {
        type_ = FileType::Directory;
    }
    else if (std::filesystem::is_regular_file(path))
    {
        type_ = FileType::Regular;
    }
    else
    {
        type_ = FileType::Unknown;
    }

    // Get file size (only for regular files)
    if (type_ == FileType::Regular)
    {
        try
        {
            size_ = std::filesystem::file_size(path);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            size_ = 0;
        }
    }

    // Get last write time
    try
    {
        lastWriteTime_ = std::filesystem::last_write_time(path);
    }
    catch (const std::filesystem::filesystem_error&)
    {
        // If we can't get the time, use current time as fallback
        lastWriteTime_ = std::filesystem::file_time_type::clock::now();
    }
}

}  // namespace fmf
