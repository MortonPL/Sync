#include "Utils.h"

#include <pwd.h>
#include <unistd.h>

std::string Utils::rootPath = "";

std::string& Utils::GetRootPath()
{
    if (Utils::rootPath == "")
    {
        auto home = getenv("HOME");
        if (home == NULL)
            home = getpwuid(getuid())->pw_dir;
        Utils::rootPath = std::string(home) + "/.sync/";
    }
    return Utils::rootPath;
}

std::string Utils::GetResourcePath()
{
    return Utils::GetRootPath() + "res/";
}

std::string Utils::GetLogsPath()
{
    return Utils::GetRootPath() + "log/";
}

std::string Utils::GetDatabasePath()
{
    return Utils::GetRootPath() + "db/";
}

std::string Utils::CorrectDirPath(const std::string& path)
{
    return path.back() != '/' ? path + '/' : path;
}

// See: https://stackoverflow.com/a/29752943
void Utils::Replace(std::string& original, const std::string& from, const std::string& to)
{
    std::string newString;
    newString.reserve(original.length());
    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while(std::string::npos != (findPos = original.find(from, lastPos)))
    {
        newString.append(original, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    newString += original.substr(lastPos);
    original.swap(newString);
}

time_t Utils::StringToTimestamp(const std::string& string)
{
    std::tm timestruct = {};
    strptime(string.c_str(), "%Y-%m-%d %H:%M:%S", &timestruct);
    return mktime(&timestruct);
}

std::string Utils::TimestampToString(const time_t& timestamp)
{
    char buf[20];
    strftime(buf, 20, "%d.%m.%Y %H:%M:%S", localtime(&timestamp));
    return std::string(buf);
}

std::string Utils::UUIDToDBPath(const uuid_t& uuid)
{
    char uuidbuf[37];
    uuid_unparse(uuid, uuidbuf);
    return std::string(uuidbuf) + ".db3";
}
