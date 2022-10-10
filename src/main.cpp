#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "headers/wx.h"
#include "gui/MainFrame.h"

// Main app
class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    wxString appPath(f.GetPath());

    wxXmlResource::Get()->InitAllHandlers();
    wxXmlResource::Get()->LoadAllFiles(appPath.Append("/rc"));

    MainFrame *pFrame = new MainFrame();
    pFrame->Show(true);
    return true;
}
