#pragma once
#include <fstream>
#include "../headers/wx.h"

class Logger
{
public:
    static void Init();
    static void Deinit();
    static void Log(std::string value);
    static void Log(wxString value);
    static void Log(const char value[]);
    static void Log(bool value);
    static void Log(int value);

private:
    static std::ofstream ofs;
};
