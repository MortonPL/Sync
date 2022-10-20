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

void Logger::Log(std::string value)
{
    Logger::ofs << value << std::endl;
    Logger::ofs.flush();
}

void Logger::Log(const char value[])
{
    Logger::ofs << value;
    Logger::ofs.flush();
}

void Logger::Log(wxString value)
{
    Logger::ofs << value.ToStdString() << std::endl;
    Logger::ofs.flush();
}

void Logger::Log(int value)
{
    Logger::ofs << value << std::endl;
    Logger::ofs.flush();
}

void Logger::Log(bool value)
{
    Logger::ofs << (value ? "true" : "false") << std::endl;
    Logger::ofs.flush();
}

void Logger::Deinit()
{
    Logger::ofs.close();
}
