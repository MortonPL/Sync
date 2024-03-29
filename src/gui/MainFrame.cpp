#include "GUI/MainFrame.h"

#include <filesystem>
#include <uuid/uuid.h>
#include <wx/busyinfo.h>

#include "GUI/Misc.h"
#include "GUI/SSHConnectorWrap.h"
#include "GUI/GenericPopup.h"
#include "GUI/NewConfigurationDialog.h"
#include "GUI/ChangeConfigurationDialog.h"
#include "GUI/GUIAnnouncer.h"
#include "GUI/ConflictRuleDialog.h"
#include "Lib/Announcer.h"
#include "Lib/DBConnector.h"
#include "Lib/Global.h"
#include "Lib/Blocker.h"
#include "Lib/Creeper.h"
#include "Lib/PairingManager.h"
#include "Lib/SyncManager.h"
#include "Lib/ConflictManager.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(XRCID("menu_file_newc"), MainFrame::OnNewConfig)
    EVT_MENU(XRCID("menu_file_changec"), MainFrame::OnChangeConfig)
    EVT_MENU(XRCID("menu_edit_scan"), MainFrame::OnScan)
    EVT_MENU(XRCID("menu_edit_sync"), MainFrame::OnSync)
    EVT_MENU(XRCID("menu_action_all"), MainFrame::OnActionSelectAll)
    EVT_MENU(XRCID("menu_action_none"), MainFrame::OnActionDeselectAll)
    EVT_MENU(XRCID("menu_action_def"), MainFrame::OnActionDefault)
    EVT_MENU(XRCID("menu_action_ltor"), MainFrame::OnActionLtoR)
    EVT_MENU(XRCID("menu_action_rtol"), MainFrame::OnActionRtoL)
    EVT_MENU(XRCID("menu_action_ignore"), MainFrame::OnActionIgnore)
    EVT_MENU(XRCID("menu_action_tool"), MainFrame::OnActionResolve)
    EVT_MENU(XRCID("menu_view_cleanon"), MainFrame::OnToggleShowClean)
    EVT_MENU(XRCID("menu_view_ffwdon"), MainFrame::OnToggleShowFastFwd)
    EVT_TOOL(XRCID("tlb_changec"), MainFrame::OnChangeConfig)
    EVT_TOOL(XRCID("tlb_scan"), MainFrame::OnScan)
    EVT_TOOL(XRCID("tlb_sync"), MainFrame::OnSync)
    EVT_TOOL(XRCID("tlb_ltor"), MainFrame::OnActionLtoR)
    EVT_TOOL(XRCID("tlb_ignore"), MainFrame::OnActionIgnore)
    EVT_TOOL(XRCID("tlb_rtol"), MainFrame::OnActionRtoL)
    EVT_TOOL(XRCID("tlb_tool"), MainFrame::OnActionResolve)
    EVT_LIST_ITEM_SELECTED(XRCID("listMain"), MainFrame::OnSelectNode)
    EVT_LIST_ITEM_DESELECTED(XRCID("listMain"), MainFrame::OnDeselectNode)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_CHAR_HOOK(MainFrame::CharHook)
wxEND_EVENT_TABLE()

#define COL_NAME        0
#define COL_STATUS_L    1
#define COL_STATUS_R    2
#define COL_ACTION      3
#define COL_PROGRESS    4

#define TOOLBAR_CONFIG  0
//      TOOLBAR_DIVIDER 1
#define TOOLBAR_SCAN    2
#define TOOLBAR_SYNC    3
//      TOOLBAR_DIVIDER 4
#define TOOLBAR_LTOR    5
#define TOOLBAR_IGNORE  6
#define TOOLBAR_RTOL    7
#define TOOLBAR_RESOLVE 8

#define MENU_FILE           0
#define MENU_EDIT           1
#define MENU_ACTION         2
#define MENU_VIEW           3
#define MENU_HELP           4

#define MENU_FILE_NEW           0
#define MENU_FILE_CHANGE        1
//      MENU_DIVIDER            2
#define MENU_FILE_EXIT          3
#define MENU_EDIT_SCAN          0
#define MENU_EDIT_SYNC          1
#define MENU_ACTION_SELECT      0
#define MENU_ACTION_DESELECT    1
//      MENU_DIVIDER            2
#define MENU_ACTION_DEFAULT     3
//      MENU_DIVIDER            4
#define MENU_ACTION_LTOR        5
#define MENU_ACTION_RTOL        6
//      MENU_DIVIDER            7
#define MENU_ACTION_IGNORE      8
//      MENU_DIVIDER            9
#define MENU_ACTION_RESOLVE     10
#define MENU_VIEW_CLEANON       0
#define MENU_VIEW_FFWDON        1
#define MENU_HELP_ABOUT         0

#define ENABLE_MENU_ITEM(menu, item)\
    this->GetMenuBar()->GetMenu(menu)->FindItemByPosition(item)->Enable();

#define ENABLE_TOOLBAR_ITEM(item, toggle)\
    toolBar->EnableTool(toolBar->GetToolByPos(item)->GetId(), toggle);

MainFrame::MainFrame(wxWindow* pParent): ssh()
{
    wxXmlResource::Get()->LoadFrame(this, pParent, "MainFrame");
    SetMenuBar(wxDynamicCast(wxXmlResource::Get()->LoadObjectRecursively(this, "MenuBar", "wxMenuBar"), wxMenuBar));
    SetStatusText("");

    ctrl = Controls
    {
        (wxListCtrl*)(FindWindow("listMain")),
        (wxTextCtrl*)(FindWindow("txtDetails")),
    };

    isFirstSelectedConfig = true;

    auto toolBar = GetToolBar();
    ENABLE_TOOLBAR_ITEM(TOOLBAR_SCAN, false);
    ENABLE_TOOLBAR_ITEM(TOOLBAR_SYNC, false);
    ENABLE_TOOLBAR_ITEM(TOOLBAR_RESOLVE, false);
    CreateColumns();
}

void MainFrame::CreateColumns()
{
    // default width available is 600
    ctrl.listMain->InsertColumn(COL_NAME, "File", 0, 320);
    ctrl.listMain->InsertColumn(COL_STATUS_L, "Local", 0, 70);
    ctrl.listMain->InsertColumn(COL_STATUS_R, "Remote", 0, 70);
    ctrl.listMain->InsertColumn(COL_ACTION, "Action", 0, 70);
    ctrl.listMain->InsertColumn(COL_PROGRESS, "Progress", 0, 70);
}

void MainFrame::PopulateList()
{
    int i = 0;
    ctrl.listMain->DeleteAllItems();
    selectedItems.clear();
    for (auto& pair: pairedNodes)
    {
        if (ShouldBeFiltered(pair))
            continue;

        ctrl.listMain->InsertItem(i, Misc::stringToWx(pair.path));
        ctrl.listMain->SetItemData(i, (long)&pair);
        auto statuses = pair.GetStatusString();
        ctrl.listMain->SetItem(i, COL_STATUS_L, statuses.first);
        ctrl.listMain->SetItem(i, COL_STATUS_R, statuses.second);
        ctrl.listMain->SetItem(i, COL_ACTION, pair.GetActionString());
        ctrl.listMain->SetItem(i, COL_PROGRESS, pair.GetProgressString());
        i++;
    }
}

void MainFrame::RefreshList()
{
    for (int i = 0; i < ctrl.listMain->GetItemCount(); i++)
    {
        auto pPair = (PairedNode*)ctrl.listMain->GetItemData(i);
        auto statuses = pPair->GetStatusString();
        ctrl.listMain->SetItem(i, COL_STATUS_L, statuses.first);
        ctrl.listMain->SetItem(i, COL_STATUS_R, statuses.second);
        ctrl.listMain->SetItem(i, COL_ACTION, pPair->GetActionString());
        ctrl.listMain->SetItem(i, COL_PROGRESS, pPair->GetProgressString());
    }
}

bool MainFrame::ShouldBeFiltered(PairedNode& pair)
{
    if (pair.defaultAction == PairedNode::Action::DoNothing && !shouldShowClean)
        return true;
    
    if (pair.defaultAction == PairedNode::Action::FastForward && !shouldShowFastFwd)
        return true;
    
    return false;
}

#define LTOA(l) wxString::Format(wxT("%ld"), l) // long to wxString
void MainFrame::ShowDetails(long itemIndex)
{
    auto writeNodeDetails = [this](const FileNode& pNode)
    {
        if (!pNode.IsEmpty())
        {
            *ctrl.txtDetails << "Status: " << FileNode::StatusAsString.at(pNode.status) << '\n';
            *ctrl.txtDetails << "Modification time: " << Utils::TimestampToString(pNode.mtime) << '\n';
            *ctrl.txtDetails << "Size: " << LTOA(pNode.size) << '\n';
            *ctrl.txtDetails << "Hash: " << pNode.HashToString() << '\n';
        }
        else
        {
            *ctrl.txtDetails << "Status: Absent\nModification time:\nSize:\nHash:\n";
        }
    };

    auto writeHistoryNodeDetails = [this](const HistoryFileNode& pNode)
    {
        if (!pNode.IsEmpty())
        {
            *ctrl.txtDetails << "Local Mtime: " << Utils::TimestampToString(pNode.mtime) << '\n';
            *ctrl.txtDetails << "Remote Mtime: " << Utils::TimestampToString(pNode.remoteMtime) << '\n';
            *ctrl.txtDetails << "Size: " << LTOA(pNode.size) << '\n';
            *ctrl.txtDetails << "Hash: " << pNode.HashToString() << '\n';
        }
        else
        {
            *ctrl.txtDetails << "Local Mtime:\nRemote Mtime:\nSize:\nHash:\n";
        }
    };

    if (itemIndex < 0 || itemIndex >= ctrl.listMain->GetItemCount())
        return;

    auto pPair = (PairedNode*)ctrl.listMain->GetItemData(itemIndex);
    ctrl.txtDetails->Clear();
    //general
    *ctrl.txtDetails << "Name: " << Misc::stringToWx(std::filesystem::path(pPair->path).filename().string()) << '\n';
    *ctrl.txtDetails << "Directory: " << Misc::stringToWx(std::filesystem::path(pPair->path).parent_path().string()) << '\n';
    *ctrl.txtDetails << "Path hash: " << pPair->pathHash << '\n';
    *ctrl.txtDetails << "Pair Action: " << pPair->GetActionString() << '\n';
    *ctrl.txtDetails << "Pair Default Action: " << pPair->GetDefaultActionString() << '\n';
    //local
    *ctrl.txtDetails << "== LOCAL ==\n";
    writeNodeDetails(pPair->localNode);
    //remote
    *ctrl.txtDetails << "== REMOTE ==\n";
    writeNodeDetails(pPair->remoteNode);
    //history
    *ctrl.txtDetails << "== HISTORY ==\n";
    writeHistoryNodeDetails(pPair->historyNode);
}
#undef LTOA

void MainFrame::UpdateItem(PairedNode* pNode, long itemIndex)
{
    auto statuses = pNode->GetStatusString();
    ctrl.listMain->SetItem(itemIndex, COL_STATUS_L, statuses.first);
    ctrl.listMain->SetItem(itemIndex, COL_STATUS_R, statuses.second);
    ctrl.listMain->SetItem(itemIndex, COL_ACTION, pNode->GetActionString());
    ctrl.listMain->SetItem(itemIndex, COL_PROGRESS, pNode->GetProgressString());
    ctrl.listMain->Refresh();
    ctrl.listMain->Update();
}

void MainFrame::OnAction(PairedNode::Action action)
{
    if (action == PairedNode::Action::None)
    {
        for (auto index: selectedItems)
        {
            PairedNode* pPair = (PairedNode*)(ctrl.listMain->GetItemData(index));
            pPair->action = pPair->defaultAction;
            ctrl.listMain->SetItem(index, COL_ACTION, pPair->GetActionString());
        }
    }
    else if (action == PairedNode::Action::Conflict || action == PairedNode::Action::Resolve)
    {
        ResolveConflict();
    }
    else
    {
        for (auto index: selectedItems)
        {
            PairedNode* pPair = (PairedNode*)(ctrl.listMain->GetItemData(index));
            if (pPair->progress == PairedNode::Progress::Canceled
                || (action != PairedNode::Action::Ignore
                    && (pPair->action == PairedNode::Action::DoNothing || pPair->action == PairedNode::Action::FastForward)))
                continue;
            pPair->action = action;
            ctrl.listMain->SetItem(index, COL_ACTION, pPair->GetActionString());
        }
    }

    if (viewedItemIndex != -1)
        ShowDetails(viewedItemIndex);
}

void MainFrame::ResolveConflict()
{
    auto cfg = Global::CurrentConfig();
    int ruleId;
    if ((ruleId = ConflictRuleDialog(conflictRules).ShowModal()) == CONFLICT_CANCEL)
        return;

    if (!SSHConnectorWrap::Connect(ssh, cfg.pathBaddress, cfg.pathBuser))
    {
        GUIAnnouncer::LogPopup("Failed to connect to the remote.", Announcer::Severity::Error);
        return;
    }
    auto sftp = SFTPConnector(&ssh);
    if (!sftp.Connect())
    {
        GUIAnnouncer::LogPopup("Failed to connect to the remote.", Announcer::Severity::Error);
        return;
    }

    auto previousCWD = std::filesystem::canonical(std::filesystem::current_path());
    std::filesystem::current_path(std::filesystem::canonical(cfg.pathA));

    std::string remoteHome;
    switch (ssh.CallCLIHome(&remoteHome))
    {
    default:
        GUIAnnouncer::LogPopup("Failed to receive remote home directory path.", Announcer::Severity::Error);
        std::filesystem::current_path(previousCWD);
        return;
    case CALLCLI_OK:
        break;
    }
    std::string tempPath = Utils::GetTempPath(remoteHome);

    if (ruleId != CONFLICT_AUTO)
    {
        for (auto index: selectedItems)
        {
            auto node = *(PairedNode*)(ctrl.listMain->GetItemData(index));
            if (node.action != PairedNode::Action::Conflict)
                continue;
            {
                wxBusyInfo wait("Fetching conflicting files. Please wait...");
                wxYield();
                if (!ConflictManager::Fetch(node, conflictRules[ruleId], cfg.pathB, tempPath, ssh, sftp))
                {
                    GUIAnnouncer::LogPopup("Failed to fetch conflicting files!", Announcer::Severity::Error);
                    continue;
                }
            }
            wxYield();
            if (!ConflictManager::Resolve(node, conflictRules[ruleId], GUIAnnouncer::LogPopup))
            {
                GUIAnnouncer::LogPopup("Failed to resolve conflict!", Announcer::Severity::Error);
                continue;
            }

            node.SetDefaultAction(PairedNode::Action::Resolve);
            ctrl.listMain->SetItem(index, COL_ACTION, node.GetActionString());
        }
    }
    else
    {
        for (auto index: selectedItems)
        {
            auto node = *(PairedNode*)(ctrl.listMain->GetItemData(index));
            if (node.action != PairedNode::Action::Conflict)
                continue;
            auto rule = ConflictRule::Match(node.path, conflictRules);
            {
                wxBusyInfo wait("Fetching conflicting files. Please wait...");
                wxYield();
                if (!ConflictManager::Fetch(node, rule, cfg.pathB, tempPath, ssh, sftp))
                {
                    GUIAnnouncer::LogPopup("Failed to fetch conflicting files!", Announcer::Severity::Error);
                    continue;
                }
            }
            wxYield();
            if (!ConflictManager::Resolve(node, rule, GUIAnnouncer::LogPopup))
            {
                GUIAnnouncer::LogPopup("Failed to resolve conflict!", Announcer::Severity::Error);
                continue;
            }

            node.SetDefaultAction(PairedNode::Action::Resolve);
            ctrl.listMain->SetItem(index, COL_ACTION, node.GetActionString());
        }
    }

    std::filesystem::current_path(previousCWD);
}

bool MainFrame::DoScan()
{
    std::forward_list<FileNode> scanNodes;
    std::forward_list<HistoryFileNode> historyNodes;
    std::forward_list<FileNode> remoteNodes;

    //get current config
    auto cfg = Global::CurrentConfig();
    LOG(INFO) << "Loaded config " << cfg.name << ".";

    //establish session
    if (!SSHConnectorWrap::Connect(ssh, cfg.pathBaddress, cfg.pathBuser))
    {
        //NOTE: SSHConnectorWrap already handles GUI popups, so we only need to log here.
        LOG(ERROR) << "Failed to connect to the remote.";
        return false;
    }
    LOG(INFO) << "Successfully connected to the remote.";

    // remove all previous data
    pairedNodes.clear();

    wxBusyInfo wait("Looking for changes. Please wait...");
    wxYield();

    // call syncli
    int rc;
    if ((rc = ssh.CallCLICreep(cfg.pathB)) != CALLCLI_OK)
    {
        GUIAnnouncer::LogPopup(fmt::format("Failed to start scan on the remote host. Error code: {}", rc), Announcer::Severity::Error);
        return false;
    }
    LOG(INFO) << "Requested remote scan.";

    // scan
    LOG(INFO) << "Begin local scan...";
    auto creeper = Creeper();
    if (!Announcer::CreeperResult(creeper.CreepPath(cfg.pathA, scanNodes), GUIAnnouncer::LogPopup))
        return false;
    LOG(INFO) << "Local scan finished.";

    // read history
    try
    {
        auto db = HistoryFileNodeDBConnector(Utils::UUIDToDBPath(cfg.uuid), SQLite::OPEN_READWRITE);
        db.SelectAll(historyNodes);
    }
    catch(const std::exception& e)
    {
        GUIAnnouncer::LogPopup("Failed to read file history.", Announcer::Severity::Error);
        return false;
    }
    LOG(INFO) << "Read file history.";

    // get remote nodes
    if ((rc = ssh.CallCLICreepReturn(remoteNodes)) != CALLCLI_OK)
    {
        GUIAnnouncer::LogPopup(fmt::format("Failed to receive file info from the remote host. Error code: {}", rc), Announcer::Severity::Error);
        return false;
    }
    LOG(INFO) << "Received remote file nodes.";

    // pairing
    LOG(INFO) << "Begin pairing...";
    Mapper mapper;
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, creeper, mapper);
    LOG(INFO) << "Pairing finished.";

    return true;
}

bool MainFrame::DoSync()
{
    LOG(INFO) << "Begin synchronizing...";
    auto previousCWD = std::filesystem::canonical(std::filesystem::current_path());
    auto cfg = Global::CurrentConfig();
    try
    {
        std::filesystem::current_path(std::filesystem::canonical(cfg.pathA));
        LOG(INFO) << "Blocking directory " << cfg.pathA;
        if (!Blocker::Block(cfg.pathA))
        {
            GUIAnnouncer::LogPopup(
                "Failed to sync files, because an entry in " + Blocker::SyncBlockedFile + " was found.\nThis can happen if a "
                "directory is already being synchronized by another instance, or if the last \n"
                "synchronization suddenly failed with no time to clean up.\nIf you are sure that "
                "nothing is being synchronized, delete " + Blocker::SyncBlockedFile + " manually.", Announcer::Severity::Error);
            return false;
        }

        auto db = HistoryFileNodeDBConnector(Utils::UUIDToDBPath(cfg.uuid), SQLite::OPEN_READWRITE);
        if (!SSHConnectorWrap::Connect(ssh, cfg.pathBaddress, cfg.pathBuser))
        {
            Blocker::Unblock(cfg.pathA);
            GUIAnnouncer::LogPopup("Failed to connect to the remote.", Announcer::Severity::Error);
            return false;
        }
        auto sftp = SFTPConnector(&ssh);
        if (!sftp.Connect())
        {
            Blocker::Unblock(cfg.pathA);
            GUIAnnouncer::LogPopup("Failed to connect to the remote.", Announcer::Severity::Error);
            return false;
        }

        std::string remoteHome;
        switch (ssh.CallCLIHomeAndBlock(cfg.pathB, &remoteHome))
        {
        case CALLCLI_BLOCKED:
            GUIAnnouncer::LogPopup("Remote is currently being synchronized with another instance.", Announcer::Severity::Error);
            Blocker::Unblock(cfg.pathA);
            std::filesystem::current_path(previousCWD);
            return false;
        default:
            GUIAnnouncer::LogPopup("Failed to receive remote home directory path.", Announcer::Severity::Error);
            Blocker::Unblock(cfg.pathA);
            std::filesystem::current_path(previousCWD);
            return false;
        case CALLCLI_OK:
            break;
        }

        std::string tempPath = Utils::GetTempPath(remoteHome);

        LOG(INFO) << "Propagating changes...";
        wxBusyInfo wait("Propagating changes. Please wait...");
        wxYield();

        if (hasSelectedEverything || selectedItems.size() == 0)
        {
            int index = 0;
            for (auto& pair: pairedNodes)
            {
                bool wasVisible = !ShouldBeFiltered(pair);
                SyncManager::Sync(&pair, cfg.pathB, tempPath, ssh, sftp, db);
                if (wasVisible)
                {
                    if (ctrl.listMain->GetItemCount() > index
                        && (PairedNode*)(ctrl.listMain->GetItemData(index)) == &pair)
                    {
                        UpdateItem(&pair, index);
                        ctrl.listMain->EnsureVisible(index);
                    }
                    index++;
                }
            }
        }
        else
        {
            for (auto index: selectedItems)
            {
                auto pNode = (PairedNode*)(ctrl.listMain->GetItemData(index));
                SyncManager::Sync(pNode, cfg.pathB, tempPath, ssh, sftp, db);
                UpdateItem(pNode, index);
                ctrl.listMain->EnsureVisible(index);
            }
        }
    }
    catch(const std::exception& e)
    {
        GUIAnnouncer::LogPopup("Failed to sync. Error: " + std::string(e.what()), Announcer::Severity::Error);
        ssh.CallCLIUnblock(cfg.pathB);
        Blocker::Unblock(cfg.pathA);
        std::filesystem::current_path(previousCWD);
        return false;
    }

    LOG(INFO) << "Unblock myself and remote...";
    ssh.CallCLIUnblock(cfg.pathB);
    Blocker::Unblock(cfg.pathA);
    std::filesystem::current_path(previousCWD);
    LOG(INFO) << "Sync finished.";

    return true;
}

/******************************* EVENT HANDLERS ******************************/

void MainFrame::OnNewConfig(wxCommandEvent& event)
{
    event.GetId(); //unused
    NewConfigurationDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
    }
}

void MainFrame::OnChangeConfig(wxCommandEvent& event)
{
    event.GetId(); //unused
    ChangeConfigurationDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
        return;
    }
    
    ssh.EndSession();

    auto toolBar = GetToolBar();
    if (isFirstSelectedConfig)
    {
        ENABLE_MENU_ITEM(MENU_EDIT, MENU_EDIT_SCAN);
        ENABLE_MENU_ITEM(MENU_ACTION, MENU_ACTION_RESOLVE);
        ENABLE_TOOLBAR_ITEM(TOOLBAR_SCAN, true);
        ENABLE_TOOLBAR_ITEM(TOOLBAR_RESOLVE, true);
        isFirstSelectedConfig = false;
    }
    ENABLE_TOOLBAR_ITEM(TOOLBAR_SYNC, false);

    ctrl.listMain->DeleteAllItems();
    pairedNodes.clear();
    selectedItems.clear();

    if (!DBConnectorStatic::EnsureCreatedHistory(Utils::UUIDToDBPath(Global::CurrentConfig().uuid)))
    {
        GenericPopup("Couldn't read configuration file history.\nThis can happen if the configuration is scanned "
                     "for the first time\nor if it's corrupted/missing. All files will be marked as new or conflicting.").ShowModal();
    }
}

void MainFrame::OnScan(wxCommandEvent& event)
{
    event.GetId(); //unused
    if (!Global::IsLoadedConfig())
        return;

    // clean everything left after the last scan
    ctrl.listMain->DeleteAllItems();
    selectedItems.clear();
    ctrl.txtDetails->Clear();

    // scan proper
    if (!DoScan())
        return;
    
    // display at the end
    pairedNodes.sort();
    PopulateList();

    auto toolBar = GetToolBar();
    ENABLE_MENU_ITEM(MENU_EDIT, MENU_EDIT_SYNC);
    ENABLE_TOOLBAR_ITEM(TOOLBAR_SYNC, true);
}

void MainFrame::OnSync(wxCommandEvent& event)
{
    event.GetId(); //unused
    // if there's nothing to sync, don't bother
    bool anythingToSync = false;
    if (hasSelectedEverything || selectedItems.size() == 0)
    {
        for (auto& pair: pairedNodes)
        {
            if (pair.action != PairedNode::Action::Conflict
                && pair.action != PairedNode::Action::DoNothing
                && pair.action != PairedNode::Action::None
                && pair.action != PairedNode::Action::Ignore
                && pair.progress != PairedNode::Progress::Canceled)
            {
                anythingToSync = true;
                break;
            }
        }
    }
    else
    {
        for (auto index: selectedItems)
        {
            auto pPair = (PairedNode*)(ctrl.listMain->GetItemData(index));
            if (pPair->action != PairedNode::Action::Conflict
                && pPair->action != PairedNode::Action::DoNothing
                && pPair->action != PairedNode::Action::None
                && pPair->action != PairedNode::Action::Ignore
                && pPair->progress != PairedNode::Progress::Canceled)
            {
                anythingToSync = true;
                break;
            }
        }
    }
    if (!anythingToSync)
    {
        return;
    }

    // sync proper
    DoSync();

    selectedItems.clear();
    viewedItemIndex = -1;
    ctrl.txtDetails->Clear();

    // update item list
    int index = 0;
    for (auto it = pairedNodes.begin(); it != pairedNodes.end();)
    {
        if (it->deleted)
        {
            if (ctrl.listMain->GetItemCount() > index
                && (PairedNode*)(ctrl.listMain->GetItemData(index)) == &*it)
            {
                ctrl.listMain->DeleteItem(index);
            }

            pairedNodes.erase(it);
            it++;
        }
        else if (ShouldBeFiltered(*it))
        {
            if (ctrl.listMain->GetItemCount() > index
                && (PairedNode*)(ctrl.listMain->GetItemData(index)) == &*it)
            {
                ctrl.listMain->DeleteItem(index);
            }

            it++;
        }
        else
        {
            UpdateItem(&*it, index);
            it++;
            index++;
        }
    }
}

void MainFrame::OnActionDefault(wxCommandEvent& event)
{
    event.GetId(); //unused
    OnAction(PairedNode::Action::None);
}

void MainFrame::OnActionLtoR(wxCommandEvent& event)
{
    event.GetId(); //unused
    OnAction(PairedNode::Action::LocalToRemote);
}

void MainFrame::OnActionIgnore(wxCommandEvent& event)
{
    event.GetId(); //unused
    OnAction(PairedNode::Action::Ignore);
}

void MainFrame::OnActionRtoL(wxCommandEvent& event)
{
    event.GetId(); //unused
    OnAction(PairedNode::Action::RemoteToLocal);
}

void MainFrame::OnActionResolve(wxCommandEvent& event)
{
    event.GetId(); //unused
    OnAction(PairedNode::Action::Conflict);
}

void MainFrame::OnSelectNode(wxListEvent& event)
{
    viewedItemIndex = event.GetIndex();
    selectedItems.insert(viewedItemIndex);
    if (selectedItems.size() == (size_t)ctrl.listMain->GetItemCount())
        hasSelectedEverything = true;

    ShowDetails(viewedItemIndex);
}

void MainFrame::OnDeselectNode(wxListEvent& event)
{
    selectedItems.erase(event.GetIndex());
    hasSelectedEverything = false;
    viewedItemIndex = -1;
    
    ctrl.txtDetails->Clear();
}

void MainFrame::OnActionSelectAll(wxCommandEvent& event)
{
    event.GetId(); //unused
    for (long i = 0; i < ctrl.listMain->GetItemCount(); i++)
    {
        ctrl.listMain->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        selectedItems.insert(i);
    }
    hasSelectedEverything = true;
}

void MainFrame::OnActionDeselectAll(wxCommandEvent& event)
{
    event.GetId(); //unused
    for (long i = 0; i < ctrl.listMain->GetItemCount(); i++)
    {
        ctrl.listMain->SetItemState(i, 0, wxLIST_STATE_SELECTED);
    }    
    selectedItems.clear();
    hasSelectedEverything = false;
}

void MainFrame::OnToggleShowClean(wxCommandEvent& event)
{
    shouldShowClean = event.IsChecked();
    PopulateList();
}

void MainFrame::OnToggleShowFastFwd(wxCommandEvent& event)
{
    shouldShowFastFwd = event.IsChecked();
    PopulateList();
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    event.GetId(); //unused
    std::string msg =
        "Sync File Synchronizer, version 1.0.0\n"
        "Author: Bartłomiej Moroz\n"
        "For more information see the README.md file distributed with the source code."
    ;

    wxMessageBox(Misc::stringToWx(msg), "About...",
                 wxOK | wxICON_INFORMATION);
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    event.GetId(); //unused
    Close(true);
}

void MainFrame::CharHook(wxKeyEvent& event)
{
    switch (event.GetKeyCode())
    {
    case WXK_LEFT:
        OnAction(PairedNode::Action::RemoteToLocal);
        break;
    case WXK_RIGHT:
        OnAction(PairedNode::Action::LocalToRemote);
        break;
    case 'S':
        if (event.GetModifiers() == wxMOD_SHIFT)
            OnAction(PairedNode::Action::Ignore);
        break;
    case 'D':
        if (event.GetModifiers() == wxMOD_SHIFT)
            OnAction(PairedNode::Action::None);
        break;
    case 'C':
        if (event.GetModifiers() == wxMOD_SHIFT)
            OnAction(PairedNode::Action::Conflict);
        break;
    case WXK_RETURN:
        if (selectedItems.size() > 0)
        {
            auto dummy = wxCommandEvent();
            OnSync(dummy);
        }
        break;
    default:
        event.DoAllowNextEvent();
        ctrl.listMain->HandleOnChar(event);
        break;
    }
}

#undef COL_NAME
#undef COL_STATUS_L
#undef COL_STATUS_R
#undef COL_ACTION
#undef COL_PROGRESS

#undef TOOLBAR_CONFIG
#undef TOOLBAR_SCAN
#undef TOOLBAR_SYNC
#undef TOOLBAR_LTOR
#undef TOOLBAR_IGNORE
#undef TOOLBAR_RTOL
#undef TOOLBAR_RESOLVE

#undef MENU_FILE
#undef MENU_EDIT
#undef MENU_ACTION
#undef MENU_VIEW
#undef MENU_HELP

#undef MENU_FILE_NEW
#undef MENU_FILE_CHANGE
#undef MENU_FILE_EXIT
#undef MENU_EDIT_SCAN
#undef MENU_EDIT_SYNC
#undef MENU_ACTION_SELECT
#undef MENU_ACTION_DESELECT
#undef MENU_ACTION_DEFAULT
#undef MENU_ACTION_LTOR
#undef MENU_ACTION_RTOL
#undef MENU_ACTION_IGNORE
#undef MENU_ACTION_RESOLVE
#undef MENU_VIEW_CLEANON
#undef MENU_VIEW_FFWDON
#undef MENU_HELP_ABOUT

#undef ENABLE_MENU_ITEM
#undef ENABLE_TOOLBAR_ITEM
