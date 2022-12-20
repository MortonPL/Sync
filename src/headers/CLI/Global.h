#pragma once
#include <string>

class GlobalCLI
{
public:
    enum CLIMode
    {
        Default = 0x000,
        DirCheck = 0x01,
        DaemonMaster = 0x010,
        DaemonServant = 0x100,
    };

    static int mode;
    static std::string remoteAddress;
    static int remotePort;
    static std::string dirToCheck;
    static std::string dirToCreep;
};
