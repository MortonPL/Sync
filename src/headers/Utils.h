#pragma once
#include "../thirdparty/easyloggingpp/easylogging++.h"
#include <fmt/core.h>
#include <string>

class Utils
{
public:
    static void Replace(std::string& original, const std::string& from, const std::string& to);

    static std::string GetDataPath();
    static std::string GetSharedPath();
    static std::string CorrectDirPath(const std::string path);

private:
    static std::string dataPath;
};
