#include "GUI/MainFrame.h"

#include <filesystem>
#include <uuid/uuid.h>

#include "GUI/SSHConnectorWrap.h"
#include "GUI/GenericPopup.h"
#include "GUI/NewConfigurationDialog.h"
#include "GUI/ChangeConfigurationDialog.h"
#include "GUI/GUIAnnouncer.h"
#include "Lib/Announcer.h"
#include "Lib/DBConnector.h"
#include "Lib/Global.h"
#include "Lib/Creeper.h"
#include "Lib/PairingManager.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(XRCID("menu_file_newc"), MainFrame::OnNewConfig)
    EVT_MENU(XRCID("menu_file_changec"), MainFrame::OnChangeConfig)
    EVT_MENU(XRCID("menu_edit_scan"), MainFrame::OnScan)
    EVT_MENU(XRCID("menu_edit_sync"), MainFrame::OnSync)
    EVT_TOOL(XRCID("tlb_changec"), MainFrame::OnChangeConfig)
    EVT_TOOL(XRCID("tlb_scan"), MainFrame::OnScan)
    EVT_TOOL(XRCID("tlb_sync"), MainFrame::OnSync)
    EVT_LIST_ITEM_SELECTED(XRCID("listMain"), MainFrame::OnSelectNode)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()

#define COL_NAME        0
#define COL_STATUS      1
#define COL_ACTION      2
#define COL_PROGRESS    3

#define TOOLBAR_CONFIG  0
//      TOOLBAR_DIVIDER 1
#define TOOLBAR_SCAN    2
#define TOOLBAR_SYNC    3

#define MENU_FILE           0
#define MENU_EDIT           1
#define MENU_HELP           2
#define MENU_FILE_NEW       0
#define MENU_FILE_CHANGE    1
#define MENU_FILE_EXIT      2
#define MENU_EDIT_SCAN      0
#define MENU_EDIT_SYNC      1
#define MENU_HELP_ABOUT     0

#define ENABLE_MENU_ITEM(menu, item)\
    this->GetMenuBar()->GetMenu(menu)->FindItemByPosition(item)->Enable();

#define ENABLE_TOOLBAR_ITEM(item, toggle)\
    toolBar->EnableTool(toolBar->GetToolByPos(item)->GetId(), toggle);

MainFrame::MainFrame(wxWindow* pParent): ssh()
{
    wxXmlResource::Get()->LoadFrame(this, pParent, "MainFrame");
    SetMenuBar(wxDynamicCast(wxXmlResource::Get()->LoadObjectRecursively(this, "MenuBar", "wxMenuBar"), wxMenuBar));
    SetStatusText("This is the status bar!");

    ctrl = Controls
    {
        (wxListCtrl*)(FindWindow("listMain")),
        (wxTextCtrl*)(FindWindow("txtDetails")),
    };

    isFirstSelectedConfig = true;

    auto toolBar = GetToolBar();
    ENABLE_TOOLBAR_ITEM(TOOLBAR_SCAN, false)
    ENABLE_TOOLBAR_ITEM(TOOLBAR_SYNC, false)
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

void MainFrame::OnNewConfig(wxCommandEvent& event)
{
    NewConfigurationDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
    }
}

void MainFrame::OnChangeConfig(wxCommandEvent& event)
{
    ChangeConfigurationDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
        return;
    }
    
    ssh.EndSession();

    if (isFirstSelectedConfig)
    {
        auto toolBar = GetToolBar();
        ENABLE_MENU_ITEM(MENU_EDIT, MENU_EDIT_SCAN)
        ENABLE_TOOLBAR_ITEM(TOOLBAR_SCAN, true)
        isFirstSelectedConfig = false;
    }

    ctrl.listMain->DeleteAllItems();

    if (DBConnector::EnsureCreatedHistory(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid)) != DB_GOOD)
    {
        GenericPopup("Configuration file history is empty.\nThis can happen if the configuration is scanned "
                     "for the first time\nor if it's corrupted. All files will be marked as new or conflicting.").ShowModal();
    }
}

void MainFrame::OnScan(wxCommandEvent& event)
{
    if (!Global::IsLoadedConfig())
        return;

    ctrl.listMain->DeleteAllItems();

    //get current config
    auto cfg = Global::GetCurrentConfig();
    LOG(INFO) << "Loaded config " << cfg.name << ".";

    //establish session
    if (!SSHConnectorWrap::Connect(ssh, cfg.pathBaddress, cfg.pathBuser))
    {
        //NOTE: SSHConnectorWrap already handles GUI popups, so we only need to log here.
        LOG(ERROR) << "Failed to connect to the remote.";
        return;
    }
    LOG(INFO) << "Successfully connected to the remote.";

    // remove all previous data
    pairedNodes.clear();

    //scan
    scanNodes.clear();
    LOG(INFO) << "Begin local scan...";
    auto creeper = Creeper();
    if (!Announcer::CreeperResult(creeper.CreepPath(cfg.pathA, scanNodes), GUIAnnouncer::LogPopup))
        return;
    LOG(INFO) << "Local scan finished.";

    //read history
    historyNodes.clear();
    try
    {
        auto db = DBConnector(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid), SQLite::OPEN_READWRITE);
        db.SelectAllFileNodes(historyNodes);
    }
    catch(const std::exception& e)
    {
        GUIAnnouncer::LogPopup("Failed to read file history.", SEV_ERROR);
        return;
    }
    LOG(INFO) << "Read file history.";

    // get remote nodes
    remoteNodes.clear();
    int rc;
    if ((rc = ssh.CallCLICreep(cfg.pathB, remoteNodes)) != CALLCLI_OK)
    {
        LOG(ERROR) << "Failed to receive file info from the remote host. Error code: " << rc;
        GenericPopup("Failed to receive file info from the remote host. Error code: " + rc).ShowModal();
        return;
    }
    LOG(INFO) << "Received remote file nodes.";

    //pairing
    LOG(INFO) << "Begin pairing...";
    Mapper mapper;
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, creeper, mapper);
    LOG(INFO) << "Pairing finished.";
    
    //display at the end
    pairedNodes.sort();
    int i = 0;
    for (auto& pair: pairedNodes)
    {
        ctrl.listMain->InsertItem(i, wxString::FromUTF8(pair.path));
        ctrl.listMain->SetItemData(i, (long)&pair);
        ctrl.listMain->SetItem(i, COL_STATUS, pair.GetStatusString());
        ctrl.listMain->SetItem(i, COL_ACTION, pair.GetActionString());
        i++;
    }

    //save data
    /*
    char uuidbuf[37];
    uuid_unparse(Global::GetCurrentConfig().uuid, uuidbuf);
    try
    {
        auto db = DBConnector(std::string(uuidbuf) + ".db3", SQLite::OPEN_READWRITE);
        for(int i = 0; i < nodes->size(); i++)
        {
            db.InsertFileNode((*nodes)[i]);
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to update file history.";
        GenericPopup("Failed to update file history.").ShowModal();
        return;
    }
    */

    auto toolBar = GetToolBar();
    ENABLE_MENU_ITEM(MENU_EDIT, MENU_EDIT_SYNC)
    ENABLE_TOOLBAR_ITEM(TOOLBAR_SYNC, true)
}

void MainFrame::OnSync(wxCommandEvent& event)
{
}

#define LTOA(l) wxString::Format(wxT("%ld"), l) // long to wxString
void MainFrame::OnSelectNode(wxListEvent& event)
{
    auto writeNodeDetails = [this](const FileNode* pNode)
    {
        if (pNode != nullptr)
        {
            *ctrl.txtDetails << "Status: " << FileNode::StatusAsString.at(pNode->status) << '\n';
            *ctrl.txtDetails << "Modification time: " << Utils::TimestampToString(pNode->mtime) << '\n';
            *ctrl.txtDetails << "Size: " << LTOA(pNode->size) << '\n';
            *ctrl.txtDetails << "Hash: " << fmt::format("{:x}{:x}", (unsigned long)pNode->hashHigh, (unsigned long)pNode->hashLow) << '\n';
        }
        else
        {
            *ctrl.txtDetails << "Status: " <<  "Absent" <<'\n';
            *ctrl.txtDetails << "Modification time: " << '\n';
            *ctrl.txtDetails << "Size: " << '\n';
            *ctrl.txtDetails << "Hash: " << '\n';
        }
    };

    auto pPair = (PairedNode*)event.GetData();
    ctrl.txtDetails->Clear();
    //general
    *ctrl.txtDetails << "Name: " << wxString::FromUTF8(std::filesystem::path(pPair->path).filename().string()) << '\n';
    *ctrl.txtDetails << "Directory: " << wxString::FromUTF8(std::filesystem::path(pPair->path).parent_path().string()) << '\n';
    *ctrl.txtDetails << "Pair Default Action: " << pPair->GetActionString() << '\n';
    //local
    *ctrl.txtDetails << "== LOCAL ==\n";
    writeNodeDetails(pPair->localNode);
    //remote
    *ctrl.txtDetails << "== REMOTE ==\n";
    writeNodeDetails(pPair->remoteNode);
    //history
    *ctrl.txtDetails << "== HISTORY ==\n";
    writeNodeDetails(pPair->historyNode);

}
#undef LTOA

void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a message box.", "About...",
                wxOK | wxICON_INFORMATION);
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

#undef COL_NAME
#undef COL_STATUS
#undef COL_ACTION
#undef COL_PROGRESS

#undef TOOLBAR_CONFIG
#undef TOOLBAR_SCAN
#undef TOOLBAR_SYNC

#undef MENU_FILE
#undef MENU_EDIT
#undef MENU_HELP
#undef MENU_FILE_NEW
#undef MENU_FILE_CHANGE
#undef MENU_FILE_EXIT
#undef MENU_EDIT_SCAN
#undef MENU_EDIT_SYNC
#undef MENU_HELP_ABOUT

#undef ENABLE_MENU_ITEM
#undef ENABLE_TOOLBAR_ITEM
