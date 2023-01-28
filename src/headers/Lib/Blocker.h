#ifndef LIB_BLOCKER_H
#define LIB_BLOCKER_H

#include <string>

namespace Blocker
{
    class BlockerException: std::exception{};
    class BlockFileException: BlockerException{};
    class AlreadyBlockedException: BlockerException{};

    void Block(const std::string& pathToBlock);
    void Block(const std::string& pathToBlock, const std::string& pathToFile);
    void Unblock(const std::string& pathToUnblock);
    void Unblock(const std::string& pathToUnblock, const std::string& pathToFile);

    extern const std::string SyncBlockedFile;
}

#endif
