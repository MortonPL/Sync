#pragma once
#include "wx.h"

#include <list>
#include <set>
#include "Lib/SSHConnector.h"
#include "domain/PairedNode.h"

class MainFrame : public wxFrame {
public:
    MainFrame(wxWindow* pParent=(wxWindow*)NULL);

private:
    // control struct
    struct Controls
    {
        wxListCtrl* listMain;
        wxTextCtrl* txtDetails;
    };

    // properties
    Controls ctrl;
    bool isFirstSelectedConfig = true;
    SSHConnector ssh;
    long viewedItemIndex = -1;
    std::set<long> selectedItems;
    bool hasSelectedEverything = false;
    bool shouldShowClean = false;
    bool shouldShowFastFwd = true;

    //temp
    std::list<PairedNode> pairedNodes;
    std::forward_list<FileNode> scanNodes;
    std::forward_list<HistoryFileNode> historyNodes;
    std::forward_list<FileNode> remoteNodes;

    // custom methods
    void CreateColumns();
    void PopulateList();
    void RefreshList();
    void OnAction(PairedNode::Action action);
    void ShowDetails(long itemIndex);
    bool ShouldBeFiltered(PairedNode& pair);

    // event handlers
    void OnNewConfig(wxCommandEvent& event);
    void OnChangeConfig(wxCommandEvent& event);
    void OnScan(wxCommandEvent& event);
    void OnSync(wxCommandEvent& event);
    void OnActionSelectAll(wxCommandEvent& event);
    void OnActionDeselectAll(wxCommandEvent& event);
    void OnActionDefault(wxCommandEvent& event);
    void OnActionLtoR(wxCommandEvent& event);
    void OnActionIgnore(wxCommandEvent& event);
    void OnActionRtoL(wxCommandEvent& event);
    void OnActionResolve(wxCommandEvent& event);
    void OnSelectNode(wxListEvent& event);
    void OnDeselectNode(wxListEvent& event);
    void OnToggleShowClean(wxCommandEvent& event);
    void OnToggleShowFastFwd(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void CharHook(wxKeyEvent& event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
