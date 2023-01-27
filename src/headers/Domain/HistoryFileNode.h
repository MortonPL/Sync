#ifndef SRC_DOMAIN_HISTORY_FILE_NODE_H
#define SRC_DOMAIN_HISTORY_FILE_NODE_H
#include "Domain/FileNode.h"

class HistoryFileNode: public FileNode
{
public:
    HistoryFileNode();
    HistoryFileNode(std::string path, dev_t dev, ino_t inode, dev_t remoteDev, ino_t remoteInode,
                    time_t mtime, time_t remoteMtime, off_t size, XXH64_hash_t hashHigh, XXH64_hash_t hashLow);

    dev_t remoteDev;
    ino_t remoteInode;
    time_t remoteMtime;

    FileNode::devinode GetRemoteDevInode() const;
};

#endif
