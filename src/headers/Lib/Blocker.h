#ifndef LIB_BLOCKER_H
#define LIB_BLOCKER_H

#include <string>

namespace Blocker
{
    bool Block(const std::string& pathToBlock);
    bool Block(const std::string& pathToBlock, const std::string& pathToFile);
    bool Unblock(const std::string& pathToUnblock);
    bool Unblock(const std::string& pathToUnblock, const std::string& pathToFile);

    extern std::string SyncBlockedFile;
}

#endif
