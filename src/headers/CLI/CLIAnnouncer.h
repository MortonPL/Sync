#pragma once

#include "Lib/Announcer.h"
#include "Utils.h"

namespace CLIAnnouncer
{
    void Log(std::string prompt, int severity=SEV_INFO)
    {
        switch (severity)
        {
        case SEV_INFO:
            LOG(INFO) << prompt;
            break;
        case SEV_WARN:
            LOG(WARNING) << prompt;
            break;
        case SEV_ERROR:
            LOG(ERROR) << prompt;
            break;
        default:
            break;
        }
    }
}
