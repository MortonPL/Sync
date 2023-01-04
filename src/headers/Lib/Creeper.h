#pragma once
#include <string>
#include <list>
#include <forward_list>
#include <filesystem>
#include <regex>
#include "xxhash.h"

#include "Domain/FileNode.h"

#define CREEP_OK 0
#define CREEP_ERROR 1
#define CREEP_PERM 2
#define CREEP_EXIST 3
#define CREEP_NOTDIR 4
#define CREEP_BLOCK 5
/*Recursively creeps through a directory and gathers FileNodes. Also provides an option to
check a path against blacklist/whitelist rules, or search for/create BLOCK files.*/
class Creeper
{
public:
    Creeper();
    ~Creeper();
    int CreepPath(std::string rootPath, std::forward_list<FileNode>& fileNodes);
    size_t GetResultsCount();
    int MakeNode(std::string& path, FileNode& node);

    bool CheckIfFileIsIgnored(std::string path);

    static std::string SyncBlackListFile;
    static std::string SyncWhiteListFile;
    static std::string SyncBlockedFile;

private:
    void SearchForLists(std::string& path);
    int MakeNode(const std::filesystem::__cxx11::directory_entry& entry,
                        std::string& rootPath, XXH3_state_t* pState, void* pBuffer, FileNode& node);
    void PreCreepCleanup(std::string& rootPath, XXH3_state_t*& pState, void*& pBuffer);
    int CheckIfDirExists(std::string& path);

    std::size_t fileNodesCount = 0;
    std::list<std::regex> whitelist;
    std::list<std::regex> blacklist;
};
