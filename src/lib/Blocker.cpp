#include "Lib/Blocker.h"

#include <fstream>
#include <filesystem>
#include "Utils.h"

const std::string Blocker::SyncBlockedFile = ".SyncBLOCKED";

void Blocker::Block(const std::string& pathToBlock, const std::string& pathToFile)
{
    std::string canonicalPath;
    try
    {
        canonicalPath = std::filesystem::canonical(pathToBlock).string();
    }
    catch(const std::exception&)
    {
        throw BlockFileException();
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
        throw AlreadyBlockedException();
    
    std::ofstream blockFileOStream(pathToFile, std::ios::app);
    blockFileOStream << canonicalPath << std::endl;
}

void Blocker::Block(const std::string& pathToBlock)
{
    Blocker::Block(pathToBlock, Utils::GetRootPath() + Blocker::SyncBlockedFile);
}

void Blocker::Unblock(const std::string& pathToUnblock, const std::string& pathToFile)
{
    std::string canonicalPath;
    try
    {
        canonicalPath = std::filesystem::canonical(pathToUnblock).string();
    }
    catch(const std::exception&)
    {
        throw BlockFileException();
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
}

void Blocker::Unblock(const std::string& pathToUnblock)
{
    Blocker::Unblock(pathToUnblock, Utils::GetRootPath() + Blocker::SyncBlockedFile);
}
