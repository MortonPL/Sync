#include "Lib/Blocker.h"

#include <filesystem>
#include <fcntl.h>
#include <stdio.h>
#include "Utils.h"

std::string Blocker::SyncBlockedFile = ".SyncBLOCKED";

Blocker::Blocker()
{
}

Blocker::~Blocker()
{
}

bool Blocker::Block(std::string pathToBlock)
{
    auto canonicalPath = std::filesystem::canonical(pathToBlock).string();
    FILE* fd;
    if ((fd = fopen((Utils::GetRootPath() + Blocker::SyncBlockedFile).c_str(), "a+")) == NULL)
        return false;
    
    char* line = NULL;
    size_t len = 0;
    bool matches = true;

    fseek(fd, 0, SEEK_SET);
    while (getline(&line, &len, fd) != -1)
    {
        if (len >= canonicalPath.size())
        {
            matches = true;
            for (int i = 0; i < canonicalPath.size(); i++)
            {
                if (line[i] != canonicalPath[i])
                {
                    matches = false;
                    break;
                }
            }
            if (matches)
            {
                fclose(fd);
                return false;
            }
        }
        else
        {
            matches = true;
            for (int i = 0; i < len; i++)
            {
                if (line[i] != canonicalPath[i])
                {
                    matches = false;
                    break;
                }
            }
            if (matches)
            {
                fclose(fd);
                return false;
            }
        }
    }

    fwrite(canonicalPath.c_str(), sizeof(char), canonicalPath.size(), fd);
    fclose(fd);
    return true;
}

bool Blocker::Unblock(std::string pathToUnblock)
{
    auto canonicalPath = std::filesystem::canonical(pathToUnblock).string();
    FILE* fd;
    FILE* fd2;
    if ((fd = fopen((Utils::GetRootPath() + Blocker::SyncBlockedFile).c_str(), "r+")) == NULL)
        return false;
    if ((fd2 = fopen((Utils::GetRootPath() + "temp").c_str(), "w+")) == NULL)
        return false;
    
    char* line = NULL;
    size_t len = 0;
    bool matches = true;

    while (getline(&line, &len, fd) != -1)
    {
        if (line == canonicalPath)
        {
            // ignore
        }
        else
        {
            fwrite(line, sizeof(char), strlen(line), fd2);
        }
    }

    fclose(fd);
    fclose(fd2);
    rename((Utils::GetRootPath() + "temp").c_str(), (Utils::GetRootPath() + Blocker::SyncBlockedFile).c_str());
    return true;
}
