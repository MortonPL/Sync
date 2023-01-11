#ifndef LIB_ANNOUNCER_H
#define LIB_ANNOUNCER_H

#include <string>

namespace Announcer
{
    enum class Severity: char
    {
        Info = 0,
        Warn,
        Error,
    };

    typedef void (*announcerType)(std::string prompt, Severity severity);

    void NoAnnouncer(const std::string prompt, Severity severity);

    bool CreeperResult(const int returnCode, announcerType announcer);
}

#endif
