#ifndef LIB_ANNOUNCER_H
#define LIB_ANNOUNCER_H

#include <string>
#include "Lib/Creeper.h"

namespace Announcer
{
    enum class Severity: char
    {
        Info = 0,
        Warn,
        Error,
    };

    typedef void (*announcerType)(std::string prompt, const Severity severity);

    void NoAnnouncer(const std::string prompt, const Severity severity);

    bool CreeperResult(const Creeper::Result result, announcerType announcer);
}

#endif
