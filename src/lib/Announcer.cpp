#include "Lib/Announcer.h"

#include "Lib/Creeper.h"
#include "Utils.h"

void Announcer::NoAnnouncer(std::string prompt, int severity){};

bool Announcer::CreeperResult(int returnCode, announcerType announcer)
{
    switch (returnCode)
    {
    case CREEP_OK:
        return true;
    case CREEP_PERM:
        announcer("Failed to scan for files in the given directory due to insufficient permissions.", SEV_ERROR);
        break;
    case CREEP_EXIST:
        announcer("Failed to scan for files, because the root directory does not exist.", SEV_ERROR);
        break;
    case CREEP_NOTDIR:
        announcer("Failed to scan for files, because the root path is not a directory.", SEV_ERROR);
        break;
    case CREEP_ERROR:
    default:
        announcer("Failed to scan for files in the given directory due to an unknown error.\nCheck logs for more information.", SEV_ERROR);
        break;
    }
    return false;
}
