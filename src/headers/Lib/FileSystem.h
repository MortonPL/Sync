#pragma once

#include <filesystem>

namespace FileSystem
{
    bool CopyLocalFile(const std::string& localPath, const std::string& tempPath, std::filesystem::copy_options options=std::filesystem::copy_options::none);
}
