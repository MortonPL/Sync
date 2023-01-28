#ifndef SRC_DOMAIN_HISTORY_FILE_NODE_H
#define SRC_DOMAIN_HISTORY_FILE_NODE_H
#include "Domain/FileNode.h"

class HistoryFileNode: public FileNode
{
public:
    HistoryFileNode();
    HistoryFileNode(const std::string path, const dev_t dev, const ino_t inode, const dev_t remoteDev, const ino_t remoteInode,
                    const time_t mtime, const time_t remoteMtime, const off_t size, const XXH64_hash_t hashHigh, const XXH64_hash_t hashLow);

    dev_t remoteDev;
    ino_t remoteInode;
    time_t remoteMtime;

    FileNode::devinode GetRemoteDevInode() const;
};

#endif
