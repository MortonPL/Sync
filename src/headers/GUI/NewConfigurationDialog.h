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
        wxTextCtrl* txtAddressB;
        wxTextCtrl* txtUserB;
        wxTextCtrl* txtRootB;
        wxButton* btnOK;
        wxButton* btnCancel;
    };

    // properties
    Controls ctrl;

    // custom methods
    void CheckIfOK();

    // event handlers
    void OnOK(wxCommandEvent &event);
    void OnAnyChange(wxCommandEvent &event);
    void OnAnyChange(wxFileDirPickerEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
