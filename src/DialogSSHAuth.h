#pragma once
#include "wx.h"

class DialogSSHAuth : public wxDialog {
public:
    DialogSSHAuth(wxWindow* pParent=(wxWindow*)NULL);
    ~DialogSSHAuth(){}

    std::string userValue;
    std::string passValue;

private:
    wxTextCtrl* pUser;
    wxTextCtrl* pPass;

    // event handlers
    void OnOK(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
