#ifndef LIB_MAPPER_H
#define LIB_MAPPER_H
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
    PairedNode* FindMapPath(std::string& path) const;
    PairedNode* FindMapLocalInode(FileNode::devinode inode) const;
    PairedNode* FindMapRemoteInode(FileNode::devinode inode) const;
    void EmplaceMapPath(const std::string& path, PairedNode& node);
    void EmplaceMapLocalInode(const FileNode::devinode inode, PairedNode& node);
    void EmplaceMapRemoteInode(const FileNode::devinode inode, PairedNode& node);
private:
    omap mapPath;
    umap mapLocalInode;
    umap mapRemoteInode;
};

#endif
