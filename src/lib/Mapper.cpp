#include "Lib/Mapper.h"

std::map<std::string, PairedNode*> Mapper::mapPath;
Mapper::umap Mapper::mapLocalInode;
Mapper::umap Mapper::mapRemoteInode;

void Mapper::Clear()
{
    mapPath.clear();
    mapLocalInode.clear();
    mapRemoteInode.clear();
}

PairedNode* Mapper::FindMapPath(std::string& path)
{
    auto it = mapPath.find(path);
    if (it == mapPath.end())
        return nullptr;
    return it->second;
}

PairedNode* Mapper::FindMapLocalInode(FileNode::devinode inode)
{
    auto it = mapLocalInode.find(inode);
    if (it == mapLocalInode.end())
        return nullptr;
    return it->second;
}

PairedNode* Mapper::FindMapRemoteInode(FileNode::devinode inode)
{
    auto it = mapRemoteInode.find(inode);
    if (it == mapRemoteInode.end())
        return nullptr;
    return it->second;
}

void Mapper::EmplaceMapPath(std::string& path, PairedNode& node)
{
    mapPath.emplace(path, &node);
}

void Mapper::EmplaceMapLocalInode(FileNode::devinode inode, PairedNode& node)
{
    mapLocalInode.emplace(inode, &node);
}

void Mapper::EmplaceMapRemoteInode(FileNode::devinode inode, PairedNode& node)
{
    mapRemoteInode.emplace(inode, &node);
}
