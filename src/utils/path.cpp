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
