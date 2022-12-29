#include "GUI/MainFrame.h"

#include <filesystem>
#include <uuid/uuid.h>

#include "GUI/SSHConnectorWrap.h"
#include "GUI/GenericPopup.h"
#include "GUI/NewConfigurationDialog.h"
#include "GUI/ChangeConfigurationDialog.h"
#include "Lib/DBConnector.h"
#include "Lib/Global.h"
#include "Lib/Creeper.h"
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
        Controls::Details
        {
            (wxStaticText*)(FindWindow("lblDetName")),
            (wxStaticText*)(FindWindow("lblDetPath")),
            (wxStaticText*)(FindWindow("lblDetDev")),
            (wxStaticText*)(FindWindow("lblDetInode")),
            (wxStaticText*)(FindWindow("lblDetMtime")),
            (wxStaticText*)(FindWindow("lblDetSize")),
            (wxStaticText*)(FindWindow("lblDetHash")),
        }
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
        GenericPopup("Configuration file history is empty.\nThis can happen if the configuration is scanned for the first time\nor if it's corrupted. All files will be marked as new or conflicting.").ShowModal();
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
    if (!ssh.IsActiveSession())
    {
        if (!SSHConnectorWrap::Connect(ssh, cfg.pathBaddress, cfg.pathBuser))
        {
            LOG(ERROR) << "Failed to connect to the remote.";
            return;
        }
    }
    LOG(INFO) << "Successfully connected to the remote.";

    //read history
    std::vector<FileNode> historyNodes;
    try
    {
        auto db = DBConnector(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid), SQLite::OPEN_READWRITE);
        historyNodes = db.SelectAllFileNodes();
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to read file history.";
        GenericPopup("Failed to read file history.").ShowModal();
        return;
    }
    LOG(INFO) << "Read file history.";
    // get remote nodes
    std::vector<FileNode> remoteNodes;
    int rc;
    if ((rc = ssh.CallCLICreep(cfg.pathB, remoteNodes)) != CALLCLI_OK)
    {
        LOG(ERROR) << "Failed to receive file info from the remote host. Error code: " << rc;
        GenericPopup("Failed to receive file info from the remote host. Error code: " + rc).ShowModal();
        return;
    }
    LOG(INFO) << "Received remote file nodes.";
    
    //scan
    LOG(INFO) << "Beginning local scan.";
    if (Creeper::CreepPath(cfg.pathA) != CREEP_OK)
    {
        LOG(ERROR) << "Failed to scan for files in the given directory.";
        GenericPopup("Failed to scan for files in the given directory.").ShowModal();
        return;
    }
    auto scanNodes = Creeper::GetResults();
    //pair history
    for(auto history: historyNodes)
    {
        auto res = Creeper::FindMapPath(history.path);
        if (res != nullptr)
        {
            res->status = STATUS_OLD;
            if (res->size == history.size)
            {
                if (res->IsEqualHash(history))
                {
                    res->status = STATUS_CLEAN;
                }
                else
                {
                    history.status = STATUS_DIRTY;
                }
            }
            else
            {
                res->status = STATUS_DIRTY;
            }
        }
        else
        {
            auto res = Creeper::FindMapInode(history.GetDevInode());
            if (res != nullptr)
            {
                if (res->IsEqualHash(history))
                {
                    res->status = STATUS_MOVED;
                    res->oldPath = history.path;
                }
                else
                {
                    history.status = STATUS_DELETED;
                    Creeper::AddNode(history);
                }
            }
            else
            {
                history.status = STATUS_DELETED;
                Creeper::AddNode(history);
            }
        }
    }
    //pair remote
    for(auto remoteNode: remoteNodes)
    {
        if (Creeper::CheckIfFileIsIgnored(remoteNode.path))
            continue;
        auto res = Creeper::FindMapPath(remoteNode.path);
        if (res != nullptr)
        {
            
        }
        else
        {
            Creeper::AddNode(remoteNode);
        }
    }
    LOG(INFO) << "Local scan finished.";
    int i = 0;
    for(auto node = (*scanNodes).begin(); node != (*scanNodes).end(); ++node)
    {
        ctrl.listMain->InsertItem(i, node->path);
        ctrl.listMain->SetItemData(i, (long)&(*node));
        ctrl.listMain->SetItem(i, COL_STATUS, FileNode::StatusString[node->status]);
        i++;
    }

    //temp
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
    auto pNode = (FileNode*)event.GetData();
    ctrl.det.lblDetName->SetLabel(std::filesystem::path(pNode->path).filename().string());
    ctrl.det.lblDetPath->SetLabel(std::filesystem::path(pNode->path).parent_path().string());
    ctrl.det.lblDetDev->SetLabel(LTOA(pNode->dev));
    ctrl.det.lblDetInode->SetLabel(LTOA(pNode->inode));
    ctrl.det.lblDetMtime->SetLabel(Utils::TimestampToString(pNode->mtime));
    ctrl.det.lblDetSize->SetLabel(LTOA(pNode->size));
    ctrl.det.lblDetHash->SetLabel(fmt::format("{:x}{:x}", (unsigned long)pNode->hashHigh, (unsigned long)pNode->hashLow));
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
