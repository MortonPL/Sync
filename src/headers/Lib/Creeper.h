#pragma once
#include <string>
#include <vector>
#include <regex>
#include "Domain/FileNode.h"

class Creeper
{
public:
    Creeper(std::string path);
    ~Creeper();

    static bool CreepPath(std::string path);
    static std::vector<FileNode>* GetResults();

    static std::map<std::string, FileNode*> mapPath;
    static std::map<FileNode::devinode, FileNode*> mapInode;

public://private:
    static std::string path;
    static std::vector<FileNode> fileNodes;
    static std::vector<std::regex> whitelist;
    static std::vector<std::regex> blacklist;

    static void SearchForLists(std::string path);
    static bool CheckBlackWhiteLists(std::string path);
};
