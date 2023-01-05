#pragma once
#include "wx.h"

#include "Domain/ConflictRule.h"

class EditConflictRuleDialog : public wxDialog
{
public:
    EditConflictRuleDialog(ConflictRule& oldRule, wxWindow* pParent=(wxWindow*)NULL);
    ~EditConflictRuleDialog(){}

private:
    // control struct
    struct Controls
    {
        wxTextCtrl* txtName;
        wxTextCtrl* txtRule;
        wxTextCtrl* txtCommand;
        wxButton* btnOK;
        wxButton* btnCancel;
    };

    // properties
    Controls ctrl;
    ConflictRule oldRule;

    // custom methods
    void CheckIfOK();

    // event handlers
    void OnOK(wxCommandEvent &event);
    void OnAnyChange(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
