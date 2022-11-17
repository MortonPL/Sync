#include "GUI/MainFrame.h"

#include "GUI/NewConfigurationDialog.h"
#include "GUI/ChangeConfigurationDialog.h"
#include "Lib/Global.h"

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
    wxXmlResource::Get()->LoadFrame(this, pParent, "MainFrame");
    SetMenuBar(wxDynamicCast(wxXmlResource::Get()->LoadObjectRecursively(this, "MenuBar", "wxMenuBar"), wxMenuBar));
    SetStatusText("This is the status bar!");

    ctrl = Controls
    {
        (wxListCtrl*)(this->FindWindow("listMain")),
    };

    // resize for menu and status bar
    //GetSizer()->SetSizeHints(this);
}

/******************************* EVENT HANDLERS ******************************/

void MainFrame::Update()
{
}

void MainFrame::OnNewConfig(wxCommandEvent &event)
{
    NewConfigurationDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
    }
}

void MainFrame::OnChangeConfig(wxCommandEvent &event)
{
    ChangeConfigurationDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
    }

    wxMessageBox(Global::config.name, "Woho", wxOK);
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
