#include "Domain/FileNode.h"

std::string FileNode::StatusString[6] =
{
    "New",
    "Deleted",
    "Clean",
    "Dirty",
    "Moved",
    "Old"
};

FileNode::FileNode(std::string path, dev_t dev, ino_t inode, time_t mtime, off_t size, XXH64_hash_t hashHigh, XXH64_hash_t hashLow)
{
    this->path = path;
    this->dev = dev;
    this->inode = inode;
    this->mtime = mtime;
    this->size = size;
    this->hashHigh = hashHigh;
    this->hashLow = hashLow;
    this->status = STATUS_NEW;
}

FileNode::~FileNode()
{
}

FileNode::devinode FileNode::GetDevInode()
{
    return devinode{dev, inode};
}

bool FileNode::IsEqualHash(const FileNode& other)
{
    return (this->hashHigh == other.hashHigh) && (this->hashLow == other.hashLow);
}
