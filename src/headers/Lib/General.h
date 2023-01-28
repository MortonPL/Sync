#ifndef LIB_GENERAL_H
#define LIB_GENERAL_H
#include <string>

/*A namespace for misc functions, i.e. app initialization.*/
namespace General
{
    bool InitEverything(const std::string logName);
    bool PreloadConfig();
    bool SaveConfig();
}

#endif
