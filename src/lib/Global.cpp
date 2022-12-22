#include "Lib/Global.h"

uuid_t uuid;

Configuration Global::config;
bool Global::hasLoadedConfig = false;
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
