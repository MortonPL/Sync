#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include "Domain/PairedNode.h"

class Mapper
{
public:
    typedef std::map<std::string, PairedNode*> omap;
    typedef std::unordered_map<FileNode::devinode, PairedNode*, FileNode::devinode::devinodeHasher> umap;

    Mapper();
    ~Mapper();

    void Clear();
    PairedNode* FindMapPath(std::string& path);
    PairedNode* FindMapLocalInode(FileNode::devinode inode);
    PairedNode* FindMapRemoteInode(FileNode::devinode inode);
    void EmplaceMapPath(const std::string& path, PairedNode& node);
    void EmplaceMapLocalInode(const FileNode::devinode inode, PairedNode& node);
    void EmplaceMapRemoteInode(const FileNode::devinode inode, PairedNode& node);
private:
    omap mapPath;
    umap mapLocalInode;
    umap mapRemoteInode;
};
