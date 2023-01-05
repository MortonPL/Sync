#include "Lib/FileSystem.h"

#include "Utils.h"

bool FileSystem::CopyLocalFile(const std::string& localPath, const std::string& tempPath, std::filesystem::copy_options options)
{
    try
    {
        std::filesystem::copy_file(localPath, tempPath, options);
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Error copying file " << localPath << ". Message: " << e.what();
        return false;
    }

    return true;
}
