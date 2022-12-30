#pragma once
#include "Domain/FileNode.h"

class HistoryFileNode: public FileNode
{
public:
    HistoryFileNode(std::string path, dev_t dev, ino_t inode, dev_t remoteDev, ino_t remoteInode,
                    time_t mtime, off_t size, XXH64_hash_t hashHigh, XXH64_hash_t hashLow);

    dev_t remoteDev;
    ino_t remoteInode;

    FileNode::devinode GetRemoteDevInode() const;
};
