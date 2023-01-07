#pragma once
#include "wx.h"
#include <vector>

#include "Domain/ConflictRule.h"

#define CONFLICT_CANCEL -1
#define CONFLICT_AUTO -2
class ConflictRuleDialog : public wxDialog {
public:
    ConflictRuleDialog(std::vector<ConflictRule>& rules, wxWindow* pParent=(wxWindow*)NULL);
    ~ConflictRuleDialog(){}

private:
    // control struct
    struct Controls
    {
        wxListBox* listConflictRules;
        wxButton* btnNew;
        wxButton* btnEdit;
        wxButton* btnMoveUp;
        wxButton* btnMoveDown;
        wxButton* btnDelete;
        wxTextCtrl* txtDetails;
        wxButton* btnAuto;
        wxButton* btnOK;
        wxButton* btnCancel;
    };

    // properties
    Controls ctrl;
    std::vector<ConflictRule>* pRules;
    long selectedRuleIdx;

    // custom methods
    void PopulateRuleList();
    void CheckIfRuleSelected();
    void PopulateRuleDetails();

    // event handlers
    void OnListBoxChange(wxCommandEvent &event);
    void OnNewRule(wxCommandEvent &event);
    void OnEditRule(wxCommandEvent &event);
    void OnMoveUpRule(wxCommandEvent &event);
    void OnMoveDownRule(wxCommandEvent &event);
    void OnDeleteRule(wxCommandEvent &event);
    void OnAuto(wxCommandEvent &event);
    void OnOK(wxCommandEvent &event);
    void OnCancel(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
