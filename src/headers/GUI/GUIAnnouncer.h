#pragma once

#include "GUI/GenericPopup.h"
#include "Lib/Announcer.h"
#include "Utils.h"

namespace GUIAnnouncer
{
    void Popup(std::string prompt, int severity=SEV_INFO)
    {
        GenericPopup(prompt).ShowModal();
    }

    void LogPopup(std::string prompt, int severity=SEV_INFO)
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
        GenericPopup(prompt).ShowModal();
    }

    void LogPopupNoBlock(std::string prompt, int severity=SEV_INFO)
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
        GenericPopup(prompt).Show();
    }
}
