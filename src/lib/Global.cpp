#include "Lib/Global.h"

Configuration Global::config;
bool Global::hasLoadedConfig = false;

bool Global::IsLoadedConfig()
{
    return Global::hasLoadedConfig;
}

const Configuration& Global::CurrentConfig()
{
    return Global::config;
}

void Global::CurrentConfig(const Configuration& config)
{
    Global::hasLoadedConfig = true;
    Global::config = config;
}
