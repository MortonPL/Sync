#pragma once

#include <string>

#define SEV_INFO 0
#define SEV_WARN 1
#define SEV_ERROR 2
namespace Announcer
{
    typedef void (*announcerType)(std::string prompt, int severity);

    void NoAnnouncer(std::string prompt, int severity);

    bool CreeperResult(int returnCode, announcerType announcer);
}
