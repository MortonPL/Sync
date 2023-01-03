#include "Domain/HistoryFileNode.h"

HistoryFileNode::HistoryFileNode()
{
}

HistoryFileNode::HistoryFileNode(std::string path, dev_t dev, ino_t inode, dev_t remoteDev, ino_t remoteInode,
                                 time_t mtime, time_t remoteMtime, off_t size, XXH64_hash_t hashHigh, XXH64_hash_t hashLow)
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
}

FileNode::devinode HistoryFileNode::GetRemoteDevInode() const
{
    return devinode{remoteDev, remoteInode};
}
