#include "Utils.h"

#include <libgen.h>
#include <unistd.h>
#include <pwd.h>
#include <linux/limits.h>

std::string Utils::dataPath = "";

std::string Utils::GetDataPath()
{
    if (Utils::dataPath == "")
    {
        auto home = getenv("HOME");
        if (home == NULL)
            home = getpwuid(getuid())->pw_dir;
        Utils::dataPath = std::string(home) + "/.sync/";
    }
    return Utils::dataPath;
}

std::string Utils::GetSharedPath()
{
    return "/usr/local/share/sync/";
}


std::string Utils::CorrectDirPath(const std::string path)
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
