#include "Lib/Blocker.h"

#include <fstream>
#include <filesystem>
#include <fcntl.h>
#include <stdio.h>
#include "Utils.h"

std::string Blocker::SyncBlockedFile = ".SyncBLOCKED";

bool Blocker::Block(const std::string& pathToBlock, const std::string& pathToFile)
{
    std::string canonicalPath;
    try
    {
        canonicalPath = std::filesystem::canonical(pathToBlock).string();
    }
    catch(const std::exception& e)
    {
        return false;
    }

    std::ifstream blockFileIStream(pathToFile);
    bool matched = false;
    bool empty = false;
    
    if (blockFileIStream.peek() == std::ifstream::traits_type::eof())
        empty = true;

    while (!empty && !matched && !blockFileIStream.eof())
    {
        std::string line;
        blockFileIStream >> line;

        if (line.size() == 0)
            continue;

        if (canonicalPath.size() > line.size())
            matched = canonicalPath.substr(0, line.size()) == line;
        else
            matched = line.substr(0, canonicalPath.size()) == canonicalPath;
    }

    if (matched)
        return false;
    
    std::ofstream blockFileOStream(pathToFile, std::ios_base::openmode::_S_app);
    blockFileOStream << canonicalPath << std::endl;

    return true;
}

bool Blocker::Block(const std::string& pathToBlock)
{
    return Blocker::Block(pathToBlock, Utils::GetRootPath() + Blocker::SyncBlockedFile);
}

bool Blocker::Unblock(const std::string& pathToUnblock, const std::string& pathToFile)
{
    std::string canonicalPath;
    try
    {
        canonicalPath = std::filesystem::canonical(pathToUnblock).string();
    }
    catch(const std::exception& e)
    {
        return false;
    }

    std::string tempFileName = pathToFile + ".tmp";
    std::ifstream blockFileIStream(pathToFile);
    std::ofstream blockFileOStream(tempFileName, std::ios_base::openmode::_S_app);
    bool matched = false;
    
    if (blockFileIStream.peek() == std::ifstream::traits_type::eof())
        return true;

    while (!matched && !blockFileIStream.eof())
    {
        std::string line;
        blockFileIStream >> line;

        if (line.size() == 0)
            continue;

        if (canonicalPath.size() > line.size())
            matched = canonicalPath.substr(0, line.size()) == line;
        else
            matched = line.substr(0, canonicalPath.size()) == canonicalPath;
        
        if (!matched)
            blockFileOStream << line << std::endl;
    }

    std::filesystem::rename(tempFileName, pathToFile);
    return true;
}

bool Blocker::Unblock(const std::string& pathToUnblock)
{
    return Blocker::Unblock(pathToUnblock, Utils::GetRootPath() + Blocker::SyncBlockedFile);
}
