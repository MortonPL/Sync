#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/filesys.h>
#include <wx/fs_arc.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "GUI/MainFrame.h"
#include "Lib/DBConnector.h"
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
    if (mkdir(Utils::GetDataPath().c_str(), S_IRWXU | S_IRWXG) == 0 || errno == EEXIST)
    {
        SetUpLogger(Utils::GetDataPath() + "syncgui.log");
        LOG(INFO) << "Starting Sync GUI.";
    }
    else
    {
        std::cout << "Failed to create application directory! Exiting.\n";
        LOG(ERROR) << "Failed to create application directory! Exiting.";
        return false;
    }

    if (!DBConnector::EnsureCreated())
    {
        std::cout << "Failed to ensure that the application database exists! Exiting.\n";
        LOG(ERROR) << "Failed to ensure that the application database exists! Exiting.";
        return false;
    }

    wxXmlResource::Get()->InitAllHandlers();
    wxImage::AddHandler(new wxPNGHandler);
    wxFileSystem::AddHandler(new wxArchiveFSHandler);
    if (!wxXmlResource::Get()->Load(Utils::GetSharedPath() + "sync.xrs"))
    {
        std::cout << "Failed to find application resources! Exiting.\n";
        LOG(ERROR) << "Failed to find application resources! Exiting.";
        return false;
    }

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
