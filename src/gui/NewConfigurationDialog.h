#pragma once
#include "../headers/wx.h"

class NewConfigurationDialog : public wxDialog {
public:
    NewConfigurationDialog(wxWindow* pParent=(wxWindow*)NULL);
    ~NewConfigurationDialog(){}

private:
    // custom
    void CheckIfOK();

    // event handlers
    void OnOK(wxCommandEvent &event);
    void OnConfigTypeChange(wxCommandEvent &event);
    void OnAnyChange(wxCommandEvent &event);
    void OnAnyChange(wxFileDirPickerEvent &event);
    // widget event table
    wxDECLARE_EVENT_TABLE();
};
