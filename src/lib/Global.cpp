#include "Lib/Global.h"

Configuration Global::config;
bool Global::hasLoadedConfig = false;

bool Global::isLoadedConfig()
{
    return Global::hasLoadedConfig;
}

const Configuration& Global::getCurrentConfig()
{
    return Global::config;
}

void Global::setCurrentConfig(const Configuration& config)
{
    Global::hasLoadedConfig = true;
    Global::config = config;
}
