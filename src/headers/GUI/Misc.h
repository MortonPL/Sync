#ifndef GUI_MISC_H
#define GUI_MISC_H

#include <string>
#include <wx.h>

namespace Misc
{
    std::string wxToString(const wxString& wx);
    wxString stringToWx(const std::string& str);
}

#endif
