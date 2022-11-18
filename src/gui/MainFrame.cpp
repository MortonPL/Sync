#include "GUI/MainFrame.h"

#include "GUI/GenericPopup.h"
#include "GUI/NewConfigurationDialog.h"
#include "GUI/ChangeConfigurationDialog.h"
#include "Lib/Global.h"
#include "Lib/Creeper.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(XRCID("menu_file_newc"), MainFrame::OnNewConfig)
    EVT_MENU(XRCID("menu_file_changec"), MainFrame::OnChangeConfig)
    EVT_MENU(XRCID("menu_edit_scan"), MainFrame::OnScan)
    EVT_TOOL(XRCID("tlb_changec"), MainFrame::OnChangeConfig)
    EVT_TOOL(XRCID("tlb_scan"), MainFrame::OnScan)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()

#define COL_LOCAL 0
#define COL_STATUS 1
#define COL_REMOTE 2

// ctor
MainFrame::MainFrame(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadFrame(this, pParent, "MainFrame");
    SetMenuBar(wxDynamicCast(wxXmlResource::Get()->LoadObjectRecursively(this, "MenuBar", "wxMenuBar"), wxMenuBar));
    SetStatusText("This is the status bar!");

    ctrl = Controls
    {
        (wxListCtrl*)(FindWindow("listMain")),
    };

    isFirstConfig = true;

    CreateReportList();

    // resize for menu and status bar
    //GetSizer()->SetSizeHints(this);
}

void MainFrame::CreateReportList()
{
    ctrl.listMain->AppendColumn("Local");
    ctrl.listMain->AppendColumn("Status");
    ctrl.listMain->AppendColumn("Remote");
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
    if (!isFirstConfig)
    {
        auto menuBar = GetMenuBar();
        auto menu = menuBar->GetMenu(menuBar->FindMenu("menu_edit"));
        menu->FindItem(menu->FindItem("menu_edit_scan"))->Enable();
        isFirstConfig = false;
    }
}

void MainFrame::OnScan(wxCommandEvent &event)
{
    if (!Global::isLoadedConfig())
        return;
    auto cfg = Global::getCurrentConfig();
    auto crp = Creeper(cfg.pathA);
    crp.SearchForLists();
    crp.CreepPath();
    auto nodes = crp.GetResults();
    for(int i = 0; i < nodes.size(); i++)
    {
        auto item = wxListItem();
        auto node = nodes[i];
        item.SetText(node.GetPath());
        item.SetId(i);
        item.SetColumn(COL_LOCAL);
        ctrl.listMain->InsertItem(item);
    }

    GenericPopup("Scanning...").ShowModal();
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

#undef COL_LOCAL
#undef COL_STATUS
#undef COL_REMOTE
