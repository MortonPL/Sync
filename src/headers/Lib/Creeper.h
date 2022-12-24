#pragma once
#include <string>
#include <vector>
#include <list>
#include <regex>

#include "Domain/FileNode.h"

/*Static class that recursively creeps through a directory and gathers FileNodes*/
class Creeper
{
public:
    static bool CreepPath(std::string rootPath);
    static std::list<FileNode>* GetResults();

    static std::map<std::string, FileNode*> mapPath;
    static std::map<FileNode::devinode, FileNode*> mapInode;

private:
    static void SearchForLists(std::string path);
    static bool CheckIfFileIsIgnored(std::string path);

    static std::list<FileNode> fileNodes;
    static std::vector<std::regex> whitelist;
    static std::vector<std::regex> blacklist;
};
