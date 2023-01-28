#include "Lib/Announcer.h"

#include "Lib/Creeper.h"

void Announcer::NoAnnouncer(const std::string, const Severity)
{
    return;
}

bool Announcer::CreeperResult(const Creeper::Result result, announcerType announcer)
{
    switch (result)
    {
    case Creeper::Result::Ok:
        return true;
    case Creeper::Result::Permissions:
        announcer("Failed to scan for files in the given directory due to insufficient permissions.", Severity::Error);
        break;
    case Creeper::Result::NotExists:
        announcer("Failed to scan for files, because the root directory does not exist.", Severity::Error);
        break;
    case Creeper::Result::NotADir:
        announcer("Failed to scan for files, because the root path is not a directory.", Severity::Error);
        break;
    case Creeper::Result::Error:
    default:
        announcer("Failed to scan for files in the given directory due to an unknown error.\nCheck logs for more information.", Severity::Error);
        break;
    }
    return false;
}
