#include "Logger.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>

std::ofstream Logger::ofs;

void Logger::Init()
{
    auto f = wxFileName(wxStandardPaths::Get().GetExecutablePath());
    auto appPath = wxString(f.GetPath());
    Logger::ofs = std::ofstream(appPath.Append("/debug.log").ToStdString(), std::ofstream::out);
    Logger::Log("Started logging.\n");
}

void Logger::Log(std::string text)
{
    Logger::ofs << text;
    Logger::ofs.flush();
}

void Logger::Deinit()
{
    Logger::ofs.close();
}
