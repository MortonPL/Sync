#pragma once
#include <fstream>

class Logger
{
public:
    static void Init();
    static void Deinit();
    static void Log(std::string text);

private:
    static std::ofstream ofs;
};
