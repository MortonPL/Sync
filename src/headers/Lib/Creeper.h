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

    void CreepPath();
    void SearchForLists();
    void GetResults();

private:
    std::string path;
    std::vector<FileNode> fileNodes;
    std::vector<std::regex> whitelist;
    std::vector<std::regex> blacklist;
};
