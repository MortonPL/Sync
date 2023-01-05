#pragma once

#include <filesystem>

class Blocker
{
public:
    Blocker();
    ~Blocker();

    bool Block(std::string pathToBlock);
    bool Unblock(std::string pathToUnblock);

    static std::string SyncBlockedFile;
};
