#pragma once

#include <filesystem>

namespace FileSystem
{
    bool CopyLocalFile(const std::string& sourcePath, const std::string& destinationPath, std::filesystem::copy_options options=std::filesystem::copy_options::none);
    bool MoveLocalFile(const std::string sourcePath, const std::string destinationPath);
}
