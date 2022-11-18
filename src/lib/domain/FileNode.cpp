#include "Domain/FileNode.h"

FileNode::FileNode(std::string path)
{
    this->path = path;
}

FileNode::~FileNode()
{
}

std::string FileNode::GetPath()
{
    return path;
}
