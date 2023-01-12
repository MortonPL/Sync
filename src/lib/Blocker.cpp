#include "Lib/Blocker.h"

#include <fstream>
#include <filesystem>
#include "Utils.h"

const std::string Blocker::SyncBlockedFile = ".SyncBLOCKED";

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
    std::string line;
    while (getline(blockFileIStream, line) && !matched)
    {
        if (line.size() == 0)
            continue;

        if (canonicalPath.size() > line.size())
            matched = canonicalPath.substr(0, line.size()) == line;
        else
            matched = line.substr(0, canonicalPath.size()) == canonicalPath;
    }

    blockFileIStream.close();

    if (matched)
        return false;
    
    std::ofstream blockFileOStream(pathToFile, std::ios::app);
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

    const std::string tempFileName = pathToFile + ".tmp";
    std::ifstream blockFileIStream(pathToFile);
    std::ofstream blockFileOStream(tempFileName, std::ios::app);
    bool matched = false;
    std::string line;
    while (getline(blockFileIStream, line) && !matched)
    {
        if (line.size() == 0)
            continue;

        if (canonicalPath.size() > line.size())
            matched = canonicalPath.substr(0, line.size()) == line;
        else
            matched = line.substr(0, canonicalPath.size()) == canonicalPath;
        
        if (!matched)
            blockFileOStream << line << std::endl;
    }

    blockFileIStream.close();
    blockFileOStream.close();

    std::filesystem::rename(tempFileName, pathToFile);
    return true;
}

bool Blocker::Unblock(const std::string& pathToUnblock)
{
    return Blocker::Unblock(pathToUnblock, Utils::GetRootPath() + Blocker::SyncBlockedFile);
}
