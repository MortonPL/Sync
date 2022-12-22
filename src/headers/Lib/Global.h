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
};
