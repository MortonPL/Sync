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
    };

    GenericPopup();
    GenericPopup(std::string message, wxWindow* pParent=(wxWindow*)NULL, std::string* output=nullptr, std::string* input=nullptr, Flags flags=Flags::None);
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
    std::string* input;
    bool isPassword;

    // event handlers
    void OnOK(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
