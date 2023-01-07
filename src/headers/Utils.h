#pragma once
#include "../thirdparty/easyloggingpp/easylogging++.h"
#include <fmt/core.h>
#include <string>
#include <uuid/uuid.h>

namespace Utils
{
    void Replace(std::string& original, const std::string& from, const std::string& to);

    void FindHomePath();

    std::string GetHomePath();
    std::string GetRootPath();
    std::string GetRootPath(std::string home);
    std::string GetResourcePath();
    std::string GetLogsPath();
    std::string GetDatabasePath();
    std::string GetTempPath();
    std::string GetTempPath(std::string home);
    
    std::string CorrectDirPath(const std::string& path);
    
    time_t StringToTimestamp(const std::string& string);
    std::string TimestampToString(const time_t& timestamp);
    std::string UUIDToDBPath(const uuid_t& uuid);
    std::string QuickHash(std::string value);
}
