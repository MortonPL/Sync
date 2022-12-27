#pragma once
#include <string>
#include <vector>
#include <list>
#include <filesystem>
#include <regex>
#include "xxhash.h"

#include "Domain/FileNode.h"

/*Static class that recursively creeps through a directory and gathers FileNodes*/
class Creeper
{
public:
    static bool CreepPath(std::string rootPath);
    static bool CreepPathNoMap(std::string rootPath);
    static std::list<FileNode>* GetResults();

    static FileNode* FindMapPath(std::string path);
    static FileNode* FindMapInode(FileNode::devinode inode);

private:
    static void SearchForLists(std::string path);
    static bool CheckIfFileIsIgnored(std::string path);
    static int MakeNode(const std::filesystem::__cxx11::directory_entry& entry,
                        std::string& rootPath, XXH3_state_t* pState, void* pBuffer, FileNode& node);
    static void PreCreepCleanup(std::string rootPath, XXH3_state_t*& pState, void*& pBuffer);

    static std::list<FileNode> fileNodes;
    static std::vector<std::regex> whitelist;
    static std::vector<std::regex> blacklist;
    static std::map<std::string, FileNode*> mapPath;
    static std::map<FileNode::devinode, FileNode*> mapInode;
};
