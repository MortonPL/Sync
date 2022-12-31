#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include "Domain/PairedNode.h"

class Mapper
{
public:
    typedef std::unordered_map<FileNode::devinode, PairedNode*, FileNode::devinode::devinodeHasher> umap;

    Mapper();
    ~Mapper();

    void Clear();
    PairedNode* FindMapPath(std::string& path);
    PairedNode* FindMapLocalInode(FileNode::devinode inode);
    PairedNode* FindMapRemoteInode(FileNode::devinode inode);
    void EmplaceMapPath(std::string& path, PairedNode& node);
    void EmplaceMapLocalInode(FileNode::devinode inode, PairedNode& node);
    void EmplaceMapRemoteInode(FileNode::devinode inode, PairedNode& node);
private:
    std::map<std::string, PairedNode*> mapPath;
    umap mapLocalInode;
    umap mapRemoteInode;
};
