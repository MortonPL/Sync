#pragma once
#include "wx.h"

#include <list>
#include <set>
#include <vector>
#include "Lib/SSHConnector.h"
#include "Domain/ConflictRule.h"
#include "Domain/PairedNode.h"

class MainFrame : public wxFrame {
public:
    MainFrame(wxWindow* pParent=nullptr);

    void PreloadConfig();

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
    std::vector<ConflictRule> conflictRules;

    // custom methods
    void CreateColumns();
    void PopulateList();
    void RefreshList();
    void OnAction(PairedNode::Action action);
    void ShowDetails(long itemIndex);
    bool ShouldBeFiltered(PairedNode& pair);
    void UpdateItem(PairedNode* pNode, long itemIndex);

    void ResolveConflict();
    bool DoScan();
    bool DoSync();

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
