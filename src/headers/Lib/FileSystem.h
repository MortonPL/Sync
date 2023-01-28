#ifndef LIB_FILESYSTEM_H
#define LIB_FILESYSTEM_H

#include <filesystem>

namespace FileSystem
{
    bool CopyLocalFile(const std::string& sourcePath, const std::string& destinationPath, const std::filesystem::copy_options options=std::filesystem::copy_options::none);
    bool MoveLocalFile(const std::string& sourcePath, const std::string& destinationPath);
}

#endif
