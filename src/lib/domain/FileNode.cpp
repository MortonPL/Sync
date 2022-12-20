#include "Domain/FileNode.h"

FileNode::FileNode(std::string path, dev_t dev, ino_t inode, time_t mtime, off_t size)
{
    this->path = path;
    this->dev = dev;
    this->inode = inode;
    this->mtime = mtime;
    this->size = size;
}

FileNode::~FileNode()
{
}

std::string FileNode::GetPath()
{
    return path;
}
