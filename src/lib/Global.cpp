#include "Lib/Global.h"

uuid_t uuid;

Configuration Global::config;
bool Global::hasLoadedConfig = false;
std::vector<FileNode> Global::nodes;
Global::LastCredsStruct Global::lastUsedCreds = {"" , "", *uuid};

bool Global::IsLoadedConfig()
{
    return Global::hasLoadedConfig;
}

const Configuration& Global::GetCurrentConfig()
{
    return Global::config;
}

void Global::SetCurrentConfig(const Configuration& config)
{
    Global::hasLoadedConfig = true;
    Global::config = config;
}

std::vector<FileNode>* Global::GetNodes()
{
    return &Global::nodes;
}

void Global::SetNodes(std::vector<FileNode> nodes)
{
    Global::nodes = nodes;
}
