#pragma once

#include "Lib/Announcer.h"
#include "Utils.h"

namespace CLIAnnouncer
{
    void Log(std::string prompt, Announcer::Severity severity=Announcer::Severity::Info)
    {
        switch (severity)
        {
        case Announcer::Severity::Info:
            LOG(INFO) << prompt;
            break;
        case Announcer::Severity::Warn:
            LOG(WARNING) << prompt;
            break;
        case Announcer::Severity::Error:
            LOG(ERROR) << prompt;
            break;
        default:
            break;
        }
    }
}
