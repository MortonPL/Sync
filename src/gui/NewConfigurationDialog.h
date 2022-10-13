#pragma once
#include "../headers/wx.h"

class NewConfigurationDialog : public wxDialog {
public:
    NewConfigurationDialog(wxWindow* pParent=(wxWindow*)NULL);
    ~NewConfigurationDialog(){}

private:
    // event handlers
    void OnOK(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
