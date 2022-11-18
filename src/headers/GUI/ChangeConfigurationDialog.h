#pragma once
#include "wx.h"
#include <vector>

#include "Domain/Configuration.h"

class ChangeConfigurationDialog : public wxDialog {
public:
    ChangeConfigurationDialog(wxWindow* pParent=(wxWindow*)NULL);
    ~ChangeConfigurationDialog(){}

private:
    // control struct
    struct Controls
    {
        wxListBox* listConfigs;
        wxButton* btnNewConfig;
        wxButton* btnEditConfig;
        wxButton* btnImportConfig;
        wxButton* btnExportConfig;
        wxButton* btnDeleteConfig;
        wxTextCtrl* txtDetails;
        wxButton* btnOK;
        wxButton* btnCancel;
    };

    // properties
    Controls ctrl;
    std::vector<Configuration> configs;
    int selectedConfigIdx;

    // custom methods
    void PopulateConfigList();
    void CheckIfConfigSelected();
    void PopulateConfigDetails();

    // event handlers
    void Update();
    void OnListBoxChange(wxCommandEvent &event);
    void OnNewConfig(wxCommandEvent &event);
    void OnEditConfig(wxCommandEvent &event);
    void OnDeleteConfig(wxCommandEvent &event);
    void OnOK(wxCommandEvent &event);
    void OnCancel(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
