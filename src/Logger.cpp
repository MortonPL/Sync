#include <fstream>
#include <wx/filename.h>
#include <wx/stdpaths.h>

void MakeSSHConnection(std::string host, std::string &res)
{
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    wxString appPath(f.GetPath());
    std::ofstream out_(appPath.Append("/ssh.log").ToStdString());
}
