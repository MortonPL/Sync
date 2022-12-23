#include "Domain/FileNode.h"

FileNode::FileNode(std::string path, dev_t dev, ino_t inode, time_t mtime, off_t size, XXH64_hash_t hashHigh, XXH64_hash_t hashLow)
{
    this->path = path;
    this->dev = dev;
    this->inode = inode;
    this->mtime = mtime;
    this->size = size;
    this->hashHigh = hashHigh;
    this->hashLow = hashLow;
}

FileNode::~FileNode()
{
}

FileNode::devinode FileNode::GetDevInode()
{
    return devinode{dev, inode};
}
