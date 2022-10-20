#pragma once
#include "../headers/wx.h"

class GenericPopup : public wxDialog {
public:
    GenericPopup();
    GenericPopup(std::string message, std::string* output=nullptr, bool isPassword=false, wxWindow* pParent=(wxWindow*)NULL);
    ~GenericPopup(){}
    GenericPopup& operator=(const GenericPopup& other);

private:
    std::string* output;

    // event handlers
    void OnOK(wxCommandEvent &event);
    // widget event table
    wxDECLARE_EVENT_TABLE();
};
