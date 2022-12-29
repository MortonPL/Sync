#pragma once
#include <string>
#include <vector>
#include <list>
#include <filesystem>
#include <regex>
#include "xxhash.h"

#include "Domain/FileNode.h"

#define CREEP_OK 0
#define CREEP_ERROR 1
#define CREEP_PERM 2
#define CREEP_EXIST 3
#define CREEP_NOTDIR 4
/*Static class that recursively creeps through a directory and gathers FileNodes*/
class Creeper
{
public:
    static int CreepPath(std::string rootPath);
    static int CreepPathNoMap(std::string rootPath);
    static std::list<FileNode>* GetResults();

    static void AddNode(FileNode& node);
    static bool CheckIfFileIsIgnored(std::string path);
    static FileNode* FindMapPath(std::string path);
    static FileNode* FindMapInode(FileNode::devinode inode);

private:
    static void SearchForLists(std::string& path);
    static int MakeNode(const std::filesystem::__cxx11::directory_entry& entry,
                        std::string& rootPath, XXH3_state_t* pState, void* pBuffer, FileNode& node);
    static void PreCreepCleanup(std::string& rootPath, XXH3_state_t*& pState, void*& pBuffer);
    static int CheckIfDirExists(std::string& path);

    static std::list<FileNode> fileNodes;
    static std::vector<std::regex> whitelist;
    static std::vector<std::regex> blacklist;
    static std::map<std::string, FileNode*> mapPath;
    static std::unordered_map<FileNode::devinode, FileNode*, FileNode::devinode::devinodeHasher> mapInode;
};
