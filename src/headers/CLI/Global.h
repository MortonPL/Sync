#pragma once
#include <string>

class GlobalCLI
{
public:
    enum CLIMode
    {
        Default = 0x000,
        HomePath = 0x010,
        DaemonServant = 0x100,
    };

    static int mode;
    static std::string dirToCreep;
    static std::string pathToStat;
    static std::string rootDir;
    static std::string dirToUnblock;
    static std::string dirToBlock;
};
