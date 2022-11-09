#pragma once
#include <string>

class Utils
{
public:
    static std::string GetProgramPath();

private:
    static bool isProgramPathSet;
    static std::string programPath;
};
