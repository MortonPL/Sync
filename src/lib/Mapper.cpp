#include "Lib/Mapper.h"

Mapper::Mapper()
{
}

Mapper::~Mapper()
{
}

void Mapper::Clear()
{
    mapPath.clear();
    mapLocalInode.clear();
    mapRemoteInode.clear();
}

PairedNode* Mapper::FindMapPath(const std::string& path) const
{
    auto it = mapPath.find(path);
    if (it == mapPath.end())
        return nullptr;
    return it->second;
}

PairedNode* Mapper::FindMapLocalInode(const FileNode::devinode inode) const
{
    auto it = mapLocalInode.find(inode);
    if (it == mapLocalInode.end())
        return nullptr;
    return it->second;
}

PairedNode* Mapper::FindMapRemoteInode(const FileNode::devinode inode) const
{
    auto it = mapRemoteInode.find(inode);
    if (it == mapRemoteInode.end())
        return nullptr;
    return it->second;
}

void Mapper::EmplaceMapPath(const std::string& path, PairedNode& node)
{
    mapPath.emplace(path, &node);
}

void Mapper::EmplaceMapLocalInode(const FileNode::devinode inode, PairedNode& node)
{
    mapLocalInode.emplace(inode, &node);
}

void Mapper::EmplaceMapRemoteInode(const FileNode::devinode inode, PairedNode& node)
{
    mapRemoteInode.emplace(inode, &node);
}
