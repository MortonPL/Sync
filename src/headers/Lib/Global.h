#ifndef LIB_GLOBAL_H
#define LIB_GLOBAL_H
#include <uuid/uuid.h>

#include "Domain/Configuration.h"
#include "Domain/ConflictRule.h"

/*A static class for globally accessible data.*/
class Global
{
public:
    static bool IsLoadedConfig();
    static const Configuration& CurrentConfig();
    static void CurrentConfig(const Configuration& config);

private:
    static Configuration config;
    static bool hasLoadedConfig;
};

#endif
