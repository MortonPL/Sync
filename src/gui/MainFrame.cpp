#include "GUI/MainFrame.h"

#include <uuid/uuid.h>

#include "GUI/SSHConnectorWrap.h"
#include "GUI/GenericPopup.h"
#include "GUI/NewConfigurationDialog.h"
#include "GUI/ChangeConfigurationDialog.h"
#include "Lib/Global.h"
#include "Lib/Creeper.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(XRCID("menu_file_newc"), MainFrame::OnNewConfig)
    EVT_MENU(XRCID("menu_file_changec"), MainFrame::OnChangeConfig)
    EVT_MENU(XRCID("menu_edit_scan"), MainFrame::OnScan)
    EVT_TOOL(XRCID("tlb_changec"), MainFrame::OnChangeConfig)
    EVT_TOOL(XRCID("tlb_scan"), MainFrame::OnScan)
    EVT_LIST_ITEM_SELECTED(XRCID("listMain"), MainFrame::OnSelectNode)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()

#define COL_NAME 0
#define COL_STATUS 1
#define COL_ACTION 2
#define COL_PROGRESS 3

// ctor
MainFrame::MainFrame(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadFrame(this, pParent, "MainFrame");
    SetMenuBar(wxDynamicCast(wxXmlResource::Get()->LoadObjectRecursively(this, "MenuBar", "wxMenuBar"), wxMenuBar));
    SetStatusText("This is the status bar!");

    ctrl = Controls
    {
        (wxListCtrl*)(FindWindow("listMain")),
        Controls::Details
        {
            (wxStaticText*)(FindWindow("lblDetName")),
            (wxStaticText*)(FindWindow("lblDetPath")),
            (wxStaticText*)(FindWindow("lblDetDev")),
            (wxStaticText*)(FindWindow("lblDetInode")),
            (wxStaticText*)(FindWindow("lblDetMtime")),
            (wxStaticText*)(FindWindow("lblDetSize")),
        }
    };

    isFirstConfig = true;

    CreateReportList();

    // resize for menu and status bar
    //GetSizer()->SetSizeHints(this);
}

void MainFrame::CreateReportList()
{
    ctrl.listMain->InsertColumn(0, "File", 0, 300);
    ctrl.listMain->InsertColumn(1, "Status", 0, 100);
    ctrl.listMain->InsertColumn(2, "Action", 0, 100);
    ctrl.listMain->InsertColumn(3, "Progress", 0, 100);
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

#define MENU_EDIT 1
#define MENU_EDIT_SCAN 0
void MainFrame::OnChangeConfig(wxCommandEvent &event)
{
    ChangeConfigurationDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
    }
    if (isFirstConfig)
    {
        GetMenuBar()->GetMenu(MENU_EDIT)->FindItemByPosition(MENU_EDIT_SCAN)->Enable();
        isFirstConfig = false;
    }
}
#undef MENU_EDIT
#undef MENU_EDIT_SCAN

void MainFrame::OnScan(wxCommandEvent &event)
{
    if (!Global::IsLoadedConfig())
        return;
    auto cfg = Global::GetCurrentConfig();
    auto crp = Creeper(cfg.pathA);
    crp.SearchForLists();
    crp.CreepPath();
    bool ok;
    /*
    auto ssh = SSHConnector();
    if (uuid_compare(cfg.uuid, Global::lastUsedCreds.uuid) == 0)
        ok = SSHConnectorWrap::Connect(ssh, cfg.pathBaddress, Global::lastUsedCreds.username, Global::lastUsedCreds.password);
    else
        ok = SSHConnectorWrap::Connect(ssh, cfg.pathBaddress, cfg.pathBuser);
    if (!ok)
        return;
    */

    Global::SetNodes(crp.GetResults());
    auto nodes = Global::GetNodes();
    ctrl.listMain->DeleteAllItems();
    for(int i = 0; i < nodes->size(); i++)
    {
        auto node = (*nodes)[i];
        ctrl.listMain->InsertItem(i, node.GetPath());
        ctrl.listMain->SetItemData(i, (long)&(*nodes)[i]);
    }

    /*
    auto nodesB = ssh.CallCLICreep(cfg.pathB);
    for(int i = 0; i < nodesB.size(); i++)
    {
        auto node = nodesB[i];
        ctrl.listMain->SetItem(i, COL_REMOTE, node.GetPath());
    }
    */
}

void MainFrame::OnSelectNode(wxListEvent &event)
{
    auto pNode = (FileNode*)event.GetData();
    ctrl.det.lblDetName->SetLabel(pNode->GetPath());
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
#undef COL_LOCAL_STATUS
#undef COL_REMOTE
#undef COL_REMOTE_STATUS
