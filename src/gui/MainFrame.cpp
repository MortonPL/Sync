#include "MainFrame.h"
#include "NewConfigurationDialog.h"
#include "ChangeConfigurationDialog.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(XRCID("menu_file_newc"), MainFrame::OnNewConfig)
    EVT_MENU(XRCID("menu_file_changec"), MainFrame::OnChangeConfig)
    EVT_TOOL(XRCID("tlb_changec"), MainFrame::OnChangeConfig)
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

    // resize for menu and status bar
    //GetSizer()->SetSizeHints(this);
}

/******************************* EVENT HANDLERS ******************************/

void MainFrame::Update()
{
}

void MainFrame::OnNewConfig(wxCommandEvent &event)
{
    auto dialog = NewConfigurationDialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {

    }
}

void MainFrame::OnChangeConfig(wxCommandEvent &event)
{
    auto dialog = ChangeConfigurationDialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {

    }
}

void MainFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox("This is a message box.", "About...",
                wxOK | wxICON_INFORMATION);
}

void MainFrame::OnExit(wxCommandEvent &event)
{
    Close(true);
}
