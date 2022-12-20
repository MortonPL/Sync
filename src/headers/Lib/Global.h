#pragma once
#include <uuid/uuid.h>
#include <vector>

#include "Domain/Configuration.h"
#include "Domain/FileNode.h"

class Global
{
public:
    static bool IsLoadedConfig();
    static const Configuration& GetCurrentConfig();
    static void SetCurrentConfig(const Configuration& config);
    static std::vector<FileNode>* GetNodes();
    static void SetNodes(std::vector<FileNode> nodes);

    struct LastCredsStruct
    {
        std::string username;
        std::string password;
        uuid_t uuid;
    };
    static LastCredsStruct lastUsedCreds;

private:
    static Configuration config;
    static bool hasLoadedConfig;
    static std::vector<FileNode> nodes;
};
