#pragma once
#include "../headers/wx.h"

class NewConfigurationDialog : public wxDialog
{
public:
    NewConfigurationDialog(wxWindow* pParent=(wxWindow*)NULL);
    ~NewConfigurationDialog(){}

private:
    // control struct
    struct Controls
    {
        wxTextCtrl* txtConfigName;
        wxDirPickerCtrl* dirRootA;
        wxChoice* ddConfigType;
        wxDirPickerCtrl* dirRootBLocal;
        wxTextCtrl* txtAddress;
        wxTextCtrl* txtUser;
        wxTextCtrl* txtRootB;
        wxButton* btnOK;
        wxButton* btnCancel;
    };

    // properties
    Controls ctrl;

    // custom methods
    void CheckIfOK();

    // event handlers
    void Update();
    void OnOK(wxCommandEvent &event);
    void OnConfigTypeChange(wxCommandEvent &event);
    void OnAnyChange(wxCommandEvent &event);
    void OnAnyChange(wxFileDirPickerEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
