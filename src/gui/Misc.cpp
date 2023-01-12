#include "GUI/Misc.h"

std::string Misc::wxToString(const wxString& wx)
{
    return std::string(wx.utf8_str());
}

wxString Misc::stringToWx(const std::string& str)
{
    return wxString::FromUTF8(str.c_str());
}
