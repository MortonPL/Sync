#pragma once
#include "wx.h"

class NewConflictRuleDialog : public wxDialog
{
public:
    NewConflictRuleDialog(wxWindow* pParent=(wxWindow*)NULL);
    ~NewConflictRuleDialog(){}

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

    // custom methods
    void CheckIfOK();

    // event handlers
    void OnOK(wxCommandEvent &event);
    void OnAnyChange(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
