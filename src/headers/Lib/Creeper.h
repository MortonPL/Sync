#pragma once
#include <string>
#include <vector>
#include "Domain/FileNode.h"

class Creeper
{
public:
    Creeper();
    ~Creeper();

    void CreepPath(std::string path);
    void GetResults();

private:
    std::vector<FileNode> fileNodes;
};
