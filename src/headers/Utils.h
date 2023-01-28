#ifndef UTILS_H
#define UTILS_H

#include "../thirdparty/easyloggingpp/easylogging++.h"
#include <fmt/core.h>
#include <string>
#include <uuid/uuid.h>

namespace Utils
{
    void Replace(std::string& string, const std::string& replaceWhat, const std::string& replaceWith);

    void FindHomePath();

    std::string GetHomePath();
    std::string GetRootPath();
    std::string GetRootPath(const std::string& home);
    std::string GetResourcePath();
    std::string GetLogsPath();
    std::string GetDatabasePath();
    std::string GetTempPath();
    std::string GetTempPath(const std::string& home);
    
    std::string CorrectDirPath(const std::string& path);
    
    time_t StringToTimestamp(const std::string& string);
    std::string TimestampToString(const time_t& timestamp);
    std::string UUIDToDBPath(const uuid_t& uuid);
    std::string QuickHash(const std::string& value);
}

#endif
