#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/filesys.h>
#include <wx/fs_arc.h>

#include "Lib/General.h"
#include "GUI/MainFrame.h"
#include "wx.h"
#include "Utils.h"

INITIALIZE_EASYLOGGINGPP

// Main app
class MyApp : public wxApp {
public:
    virtual bool OnInit();
    virtual int OnExit();
private:
    MainFrame* pMainFrame;
};

wxIMPLEMENT_APP(MyApp);

bool InitXRC()
{
    wxXmlResource::Get()->InitAllHandlers();
    wxImage::AddHandler(new wxPNGHandler);
    wxFileSystem::AddHandler(new wxArchiveFSHandler);
    return wxXmlResource::Get()->Load(Utils::GetResourcePath() + "sync.xrs");
}

bool MyApp::OnInit()
{
    if (!General::InitEverything("syncgui.log"))
        return false;

    if (!InitXRC())
    {
        std::cout << "Failed to find application resources! Exiting.\n";
        LOG(ERROR) << "Failed to find application resources! Exiting.";
        return false;
    }

    pMainFrame = new MainFrame();
    if (General::PreloadConfig())
        pMainFrame->PreloadConfig();
    pMainFrame->Show(true);
    return true;
}

int MyApp::OnExit()
{
    General::SaveConfig();
    //pMainFrame->Destroy();
    LOG(INFO) << "Exiting.";
    return 0;
}
