#pragma once
#include "../headers/wx.h"

class GenericPopup : public wxDialog {
public:
    GenericPopup();
    GenericPopup(std::string message, std::string* output=nullptr, bool isPassword=false, wxWindow* pParent=(wxWindow*)NULL);
    ~GenericPopup(){}

private:
    // control struct
    struct Controls
    {
        wxTextCtrl* txtInputStd;
        wxTextCtrl* txtInputPass;
        wxButton* btnOK;
    };

    // properties
    Controls ctrl;
    std::string* output;
    bool isPassword;

    // event handlers
    void OnOK(wxCommandEvent &event);
    // widget event table
    wxDECLARE_EVENT_TABLE();
};
