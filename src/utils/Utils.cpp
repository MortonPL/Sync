#include "Utils.h"

#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>

std::string Utils::programPath;
bool Utils::isProgramPathSet = false;

std::string Utils::GetProgramPath()
{
    if (Utils::isProgramPathSet)
    {
        return Utils::programPath;
    }
    else
    {
        char result[PATH_MAX];
        if (readlink("/proc/self/exe", result, PATH_MAX) == -1)
            return "";
        Utils::programPath = std::string(dirname(result)) + "/";
        Utils::isProgramPathSet = true;
        return Utils::programPath;
    }
}

std::string Utils::CorrectDirPath(const std::string path)
{
    if (path.back() != '/')
        return path + '/';
    return path;
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
