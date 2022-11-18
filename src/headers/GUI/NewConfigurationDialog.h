#pragma once
#include "wx.h"

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
        wxTextCtrl* txtAddressB;
        wxTextCtrl* txtUserB;
        wxTextCtrl* txtRootB;
        wxTextCtrl* txtAddressA;
        wxTextCtrl* txtUserA;
        wxButton* btnOK;
        wxButton* btnCancel;
    };

    // properties
    Controls ctrl;

    // custom methods
    void CheckIfOK();
    void EnableRemote();
    void DisableRemote();

    // event handlers
    void Update();
    void OnOK(wxCommandEvent &event);
    void OnConfigTypeChange(wxCommandEvent &event);
    void OnAnyChange(wxCommandEvent &event);
    void OnAnyChange(wxFileDirPickerEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
