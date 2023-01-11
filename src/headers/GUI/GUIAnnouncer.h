#pragma once

#include "GUI/GenericPopup.h"
#include "Lib/Announcer.h"
#include "Utils.h"

namespace GUIAnnouncer
{
    void Popup(std::string prompt, Announcer::Severity)
    {
        GenericPopup(prompt).ShowModal();
    }

    void LogPopup(std::string prompt, Announcer::Severity severity=Announcer::Severity::Info)
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
        GenericPopup(prompt).ShowModal();
    }

    void LogPopupNoBlock(std::string prompt, Announcer::Severity severity=Announcer::Severity::Info)
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
        GenericPopup(prompt).Show();
    }
}
