#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "headers/wx.h"
#include "gui/MainFrame.h"
#include "utils/Logger.h"

// Main app
class MyApp : public wxApp {
public:
    virtual bool OnInit();
    virtual int OnExit();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    Logger::Init();
    wxImage::AddHandler(new wxPNGHandler);
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    wxString appPath(f.GetPath());

    wxXmlResource::Get()->InitAllHandlers();
    wxXmlResource::Get()->LoadAllFiles(appPath.Append("/rc"));

    MainFrame *pFrame = new MainFrame();
    pFrame->Show(true);
    return true;
}

int MyApp::OnExit()
{
    Logger::Deinit();
    return 0;
}
