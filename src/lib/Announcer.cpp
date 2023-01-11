#include "Lib/Announcer.h"

#include "Lib/Creeper.h"
#include "Utils.h"

void Announcer::NoAnnouncer(const std::string, Severity)
{
    return;
}

bool Announcer::CreeperResult(const int returnCode, announcerType announcer)
{
    switch (returnCode)
    {
    case CREEP_OK:
        return true;
    case CREEP_PERM:
        announcer("Failed to scan for files in the given directory due to insufficient permissions.", Severity::Error);
        break;
    case CREEP_EXIST:
        announcer("Failed to scan for files, because the root directory does not exist.", Severity::Error);
        break;
    case CREEP_NOTDIR:
        announcer("Failed to scan for files, because the root path is not a directory.", Severity::Error);
        break;
    case CREEP_ERROR:
    default:
        announcer("Failed to scan for files in the given directory due to an unknown error.\nCheck logs for more information.", Severity::Error);
        break;
    }
    return false;
}
