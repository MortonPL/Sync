#include "MainFrame.h"
#include "DialogSSHAuth.h"
#include "../lib/SSHConnector.h"
#include "../utils/Logger.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()

// ctor
MainFrame::MainFrame(wxWindow* pParent)
{
    auto wxXml = wxXmlResource::Get();

    wxXml->LoadFrame(this, pParent, "MainFrame");

    SetMenuBar(wxDynamicCast(wxXml->LoadObjectRecursively(this, "MenuBar", "wxMenuBar"), wxMenuBar));
    SetStatusText("This is the status bar!");

    // load panels (temp)
    auto pPanelSizer = GetSizer();
    allPanels.push_back(wxXml->LoadPanel(this, "DirectoryViewPanel"));
    pLeftPanel = allPanels[0];
    pLeftPanel->Show();
    pPanelSizer->Add(pLeftPanel);
    pPanelSizer->Layout();

    // resize for menu and status bar
    GetSizer()->SetSizeHints(this);
}

/********************************* RESOURCES *********************************/

/******************************* EVENT HANDLERS ******************************/

void MainFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox("This is a message box.", "About...",
                wxOK | wxICON_INFORMATION);
}

void MainFrame::OnExit(wxCommandEvent &event)
{
    Close(true);
}
