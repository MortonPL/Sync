#include "Utils.h"

#include <pwd.h>
#include <unistd.h>
#include "xxhash.h"

std::string homePath;

void Utils::FindHomePath()
{
    if (homePath.length() <= 0)
    {
        char* home = getenv("HOME");
        if (home == nullptr)
            home = getpwuid(getuid())->pw_dir;
        homePath = home;
    }
}

std::string Utils::GetHomePath()
{
    return homePath;
}

std::string Utils::GetRootPath()
{
    return Utils::GetHomePath() + "/.sync/";
}

std::string Utils::GetRootPath(const std::string& home)
{
    return home + "/.sync/";
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

std::string Utils::GetTempPath()
{
    return Utils::GetRootPath() + "tmp/";
}

std::string Utils::GetTempPath(const std::string& home)
{
    return Utils::GetRootPath(home) + "tmp/";
}

std::string Utils::CorrectDirPath(const std::string& path)
{
    return path.back() != '/' ? path + '/' : path;
}

// A simple extra utility to inplace replace multiple characters in a string
// See: https://stackoverflow.com/a/29752943
void Utils::Replace(std::string& string, const std::string& replaceWhat, const std::string& replaceWith)
{
    std::string newString;
    newString.reserve(string.length());
    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while(std::string::npos != (findPos = string.find(replaceWhat, lastPos)))
    {
        newString.append(string, lastPos, findPos - lastPos);
        newString += replaceWith;
        lastPos = findPos + replaceWhat.length();
    }

    newString += string.substr(lastPos);
    string.swap(newString);
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

std::string Utils::QuickHash(const std::string& value)
{
    return fmt::format("{:x}", XXH64(value.c_str(), value.size(), 0));
}
