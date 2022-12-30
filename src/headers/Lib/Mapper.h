#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include "Domain/PairedNode.h"

class Mapper
{
public:
    typedef std::unordered_map<FileNode::devinode, PairedNode*, FileNode::devinode::devinodeHasher> umap;

    static void Clear();
    static PairedNode* FindMapPath(std::string& path);
    static PairedNode* FindMapLocalInode(FileNode::devinode inode);
    static PairedNode* FindMapRemoteInode(FileNode::devinode inode);
    static void EmplaceMapPath(std::string& path, PairedNode& node);
    static void EmplaceMapLocalInode(FileNode::devinode inode, PairedNode& node);
    static void EmplaceMapRemoteInode(FileNode::devinode inode, PairedNode& node);
private:
    static std::map<std::string, PairedNode*> mapPath;
    static umap mapLocalInode;
    static umap mapRemoteInode;
};
