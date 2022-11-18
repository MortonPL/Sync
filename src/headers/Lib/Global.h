#pragma once
#include "Domain/Configuration.h"

class Global
{
public:
    static const Configuration& getCurrentConfig();
    static void setCurrentConfig(const Configuration& config);
    static bool isLoadedConfig();

private:
    static Configuration config;
    static bool hasLoadedConfig;
};
