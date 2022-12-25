#pragma once
#include "wx.h"

class GenericPopup : public wxDialog {
public:
    GenericPopup();
    GenericPopup(std::string message, wxWindow* pParent=(wxWindow*)NULL, std::string* output=nullptr, bool isPassword=false, bool canBeCanceled=false);
    ~GenericPopup(){}

private:
    // control struct
    struct Controls
    {
        wxTextCtrl* txtInputStd;
        wxTextCtrl* txtInputPass;
        wxButton* btnOK;
        wxButton* btnCancel;
    };

    // properties
    Controls ctrl;
    std::string* output;
    bool isPassword;
    bool canBeCanceled;

    // event handlers
    void OnOK(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
