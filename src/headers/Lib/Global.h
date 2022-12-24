#pragma once
#include <uuid/uuid.h>

#include "Domain/Configuration.h"
#include "Domain/FileNode.h"

/*A static class for globally accessible data.*/
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
