#pragma once
#include "../thirdparty/easyloggingpp/easylogging++.h"
#include <fmt/core.h>
#include <string>
#include <iomanip>

class Utils
{
public:
    static void Replace(std::string& original, const std::string& from, const std::string& to);

    static std::string GetRootPath();
    static std::string GetResourcePath();
    static std::string GetLogsPath();
    static std::string GetDatabasePath();
    static std::string CorrectDirPath(const std::string path);
    static time_t StringToTimestamp(const std::string string);
    static std::string TimestampToString(const time_t* timestamp);
private:
    static std::string dataPath;
};
