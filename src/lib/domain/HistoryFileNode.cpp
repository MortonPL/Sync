#include "Domain/HistoryFileNode.h"

HistoryFileNode::HistoryFileNode()
{
}

HistoryFileNode::HistoryFileNode(const std::string path, const dev_t dev, const ino_t inode, const dev_t remoteDev, const ino_t remoteInode,
                                 const time_t mtime, const time_t remoteMtime, const off_t size, const XXH64_hash_t hashHigh, const XXH64_hash_t hashLow)
{
    this->path = path;
    this->dev = dev;
    this->inode = inode;
    this->remoteDev = remoteDev;
    this->remoteInode = remoteInode;
    this->mtime = mtime;
    this->remoteMtime = remoteMtime;
    this->size = size;
    this->hashHigh = hashHigh;
    this->hashLow = hashLow;
    this->status = FileNode::Status::HistoryPresent;
    this->noHash = false;
}

FileNode::devinode HistoryFileNode::GetRemoteDevInode() const
{
    return devinode{remoteDev, remoteInode};
}
