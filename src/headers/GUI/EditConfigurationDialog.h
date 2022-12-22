#pragma once
#include "wx.h"

#include "Domain/Configuration.h"

class EditConfigurationDialog : public wxDialog
{
public:
    EditConfigurationDialog(Configuration& oldConfig, wxWindow* pParent=(wxWindow*)NULL);
    ~EditConfigurationDialog(){}

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
    Configuration oldConfig;

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