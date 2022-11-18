#pragma once
#include "wx.h"

class GenericPopup : public wxDialog {
public:
    GenericPopup();
    GenericPopup(std::string message, wxWindow* pParent=(wxWindow*)NULL, std::string* output=nullptr, bool isPassword=false);
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
    void Update();
    void OnOK(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
