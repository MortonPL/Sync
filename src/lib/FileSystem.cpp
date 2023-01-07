#include "Lib/FileSystem.h"

#include "Utils.h"

bool FileSystem::CopyLocalFile(const std::string& sourcePath, const std::string& destinationPath, std::filesystem::copy_options options)
{
    try
    {
        std::filesystem::copy_file(sourcePath, destinationPath, options);
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Error copying file " << sourcePath << ". Message: " << e.what();
        return false;
    }

    return true;
}

bool FileSystem::MoveLocalFile(const std::string sourcePath, const std::string destinationPath)
{
    // create parent directories if needed
    auto path = std::filesystem::path(destinationPath);
    if (path.has_parent_path())
    {
        try
        {
            std::filesystem::create_directories(path.parent_path());
        }
        catch(const std::exception& e)
        {
            LOG(ERROR) << "Failed to create directory " << path.parent_path() << " Reason: " << e.what() << '\n';
            return false;
        }
    }
    // try to move atomically, if it fails, copy the old fashioned way
    if (rename(sourcePath.c_str(), destinationPath.c_str()) < 0)
    {
        // error!
        int err = errno;
        LOG(WARNING) << "Error moving atomically temporary file " << sourcePath << ". Message: " << strerror(err);
        if (err == EXDEV)
        {
            if (FileSystem::CopyLocalFile(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing))
                remove(sourcePath.c_str());
            else
                return false;
        }
        else
        {
            return false;
        }
    }

    return true;
}
