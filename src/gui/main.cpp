#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "GUI/MainFrame.h"
#include "Lib/DBConnector.h"
#include "Utils/Utils.h"
#include "wx.h"
#include "Utils/Logger.h"

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

void SetUpLogger(std::string logPath)
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    //defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime %level %msg");
    // To set GLOBAL configurations you may use
    defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");
    defaultConf.setGlobally(el::ConfigurationType::Filename, logPath);
    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
    el::Loggers::reconfigureLogger("default", defaultConf);
}

bool MyApp::OnInit()
{
    SetUpLogger(Utils::GetProgramPath() + "SyncGUI.log");
    LOG(INFO) << "Starting Sync GUI.";

    DBConnector::EnsureCreated();

    wxImage::AddHandler(new wxPNGHandler);

    wxXmlResource::Get()->InitAllHandlers();
    wxXmlResource::Get()->LoadAllFiles(Utils::GetProgramPath() + "rc");

    auto pMainFrame = new MainFrame();
    pMainFrame->Show(true);
    return true;
}

int MyApp::OnExit()
{
    delete pMainFrame;
    LOG(INFO) << "Exiting.";
    return 0;
}
