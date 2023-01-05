#pragma once
#include <uuid/uuid.h>

#include "Domain/Configuration.h"
#include "Domain/ConflictRule.h"

/*A static class for globally accessible data.*/
class Global
{
public:
    static bool IsLoadedConfig();
    static const Configuration& GetCurrentConfig();
    static void SetCurrentConfig(const Configuration& config);

private:
    static Configuration config;
    static bool hasLoadedConfig;
};
