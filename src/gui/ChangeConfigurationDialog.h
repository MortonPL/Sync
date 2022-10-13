#pragma once
#include "../headers/wx.h"

class ChangeConfigurationDialog : public wxDialog {
public:
    ChangeConfigurationDialog(wxWindow* pParent=(wxWindow*)NULL);
    ~ChangeConfigurationDialog(){}

private:
    // event handlers
    void OnOK(wxCommandEvent &event);
    void OnCancel(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
