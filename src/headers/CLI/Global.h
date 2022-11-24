#pragma once
#include <string>

class Global
{
public:
    enum CLIMode
    {
        Default = 0,
        DirCheck,
        Test,
    };

    static CLIMode mode;
    static std::string remoteAddress;
    static std::string remoteUser;
    static std::string remoteRoot;
    static std::string dirToCheck;
};
