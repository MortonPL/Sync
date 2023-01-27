#pragma once
#include "wx.h"

#include "Domain/Configuration.h"

class EditConfigurationDialog : public wxDialog
{
public:
    EditConfigurationDialog(Configuration& oldConfig, wxWindow* pParent=nullptr);
    ~EditConfigurationDialog(){}

private:
    // control struct
    struct Controls
    {
        wxTextCtrl* txtConfigName;
        wxDirPickerCtrl* dirRootA;
        wxTextCtrl* txtAddressB;
        wxTextCtrl* txtUserB;
        wxTextCtrl* txtRootB;
        wxButton* btnOK;
        wxButton* btnCancel;
    };

    // properties
    Controls ctrl;
    Configuration oldConfig;

    // custom methods
    void CheckIfOK();

    // event handlers
    void OnOK(wxCommandEvent &event);
    void OnAnyChange(wxCommandEvent &event);
    void OnAnyChange(wxFileDirPickerEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
