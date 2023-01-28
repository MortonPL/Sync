#pragma once
#include "wx.h"

class GenericPopup : public wxDialog {
public:
    enum Flags: unsigned char
    {
        None = 0x0,
        Password = 0x1,
        Cancel = 0x2,
        PasswordCancel = 0x3,
        NoButtons = 0x4,
    };

    GenericPopup();
    GenericPopup(std::string message, wxWindow* pParent=nullptr, std::string* output=nullptr, const std::string* input=nullptr, Flags flags=Flags::None);
    ~GenericPopup(){}

private:
    // control struct
    struct Controls
    {
        wxTextCtrl* txtInputStd;
        wxTextCtrl* txtInputPass;
        wxTextCtrl* txtOutput;
        wxButton* btnOK;
        wxButton* btnCancel;
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
