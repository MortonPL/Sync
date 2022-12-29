#include "GUI/ChangeConfigurationDialog.h"

#include "GUI/NewConfigurationDialog.h"
#include "GUI/EditConfigurationDialog.h"
#include "GUI/GenericPopup.h"
#include "Lib/DBConnector.h"
#include "Lib/Global.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(ChangeConfigurationDialog, wxDialog)
    EVT_LISTBOX(XRCID("listConfigs"), ChangeConfigurationDialog::OnListBoxChange)
    EVT_LISTBOX_DCLICK(XRCID("listConfigs"), ChangeConfigurationDialog::OnOK)
    EVT_BUTTON(XRCID("btnNewConfig"), ChangeConfigurationDialog::OnNewConfig)
    EVT_BUTTON(XRCID("btnEditConfig"), ChangeConfigurationDialog::OnEditConfig)
    EVT_BUTTON(XRCID("btnDeleteConfig"), ChangeConfigurationDialog::OnDeleteConfig)
    EVT_BUTTON(wxID_OK, ChangeConfigurationDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ChangeConfigurationDialog::OnCancel)
wxEND_EVENT_TABLE()

// ctor
ChangeConfigurationDialog::ChangeConfigurationDialog(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "ChangeConfigurationDialog");

    ctrl = Controls
    {
        (wxListBox*)(FindWindow("listConfigs")),
        (wxButton*)(FindWindow("btnNewConfig")),
        (wxButton*)(FindWindow("btnEditConfig")),
        (wxButton*)(FindWindow("btnImportConfig")),
        (wxButton*)(FindWindow("btnExportConfig")),
        (wxButton*)(FindWindow("btnDeleteConfig")),
        (wxTextCtrl*)(FindWindow("txtDetails")),
        (wxButton*)(FindWindow("wxID_OK")),
        (wxButton*)(FindWindow("wxID_CANCEL")),
    };

    this->selectedConfigIdx = -1;
    PopulateConfigList();
}

/*********************************** OTHER ***********************************/

void ChangeConfigurationDialog::PopulateConfigList()
{
    DBConnector db(DBConnector::GetMainFileName());
    try
    {
        this->configs = db.SelectAllConfigs();
        auto arrs = wxArrayString();
        for(auto config: this->configs)
            arrs.Add(config.name);
        ctrl.listConfigs->Clear();
        if (!arrs.IsEmpty())
            ctrl.listConfigs->InsertItems(arrs, 0);
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        GenericPopup("Failed to open/refresh configuration database.").ShowModal();
        return;
    }
}

void ChangeConfigurationDialog::PopulateConfigDetails()
{
    if (this->selectedConfigIdx < 0)
        return;

    auto config = this->configs[this->selectedConfigIdx];
    ctrl.txtDetails->Clear();
    *ctrl.txtDetails << "General info:\n";
    *ctrl.txtDetails << "\tName: " << config.name << "\n";
    *ctrl.txtDetails << "\tPath A: " << config.pathA << "\n";
    *ctrl.txtDetails << "\tPath B: " << config.pathB << "\n";
    *ctrl.txtDetails << "\tLast config edit: " << Utils::TimestampToString(config.timestamp) << "\n";
    *ctrl.txtDetails << "\tPath B Address/Hostname: " << config.pathBaddress << "\n";
    *ctrl.txtDetails << "\tPath B User: " << config.pathBuser << "\n";
}

void ChangeConfigurationDialog::CheckIfConfigSelected()
{
    selectedConfigIdx = ctrl.listConfigs->GetSelection();
    if (selectedConfigIdx != wxNOT_FOUND)
    {
        PopulateConfigDetails();
        ctrl.btnEditConfig->Enable();
        ctrl.btnExportConfig->Enable();
        ctrl.btnDeleteConfig->Enable();
        ctrl.btnOK->Enable();
    }
    else
    {
        ctrl.txtDetails->Clear();
        ctrl.btnEditConfig->Disable();
        ctrl.btnExportConfig->Disable();
        ctrl.btnDeleteConfig->Disable();
        ctrl.btnOK->Disable();
    }
}

/******************************* EVENT HANDLERS ******************************/

void ChangeConfigurationDialog::Update()
{
}

void ChangeConfigurationDialog::OnListBoxChange(wxCommandEvent &event)
{
    CheckIfConfigSelected();
    Update();
}

void ChangeConfigurationDialog::OnNewConfig(wxCommandEvent &event)
{
    NewConfigurationDialog().ShowModal();
    PopulateConfigList();
    Update();
}

void ChangeConfigurationDialog::OnEditConfig(wxCommandEvent &event)
{
    if (EditConfigurationDialog(configs[selectedConfigIdx]).ShowModal() == wxID_OK)
    {
        PopulateConfigList();
        PopulateConfigDetails();
        Update();
    }
}

void ChangeConfigurationDialog::OnDeleteConfig(wxCommandEvent &event)
{
    try
    {
        DBConnector db(DBConnector::GetMainFileName(), SQLite::OPEN_READWRITE);
        if (db.DeleteConfig(configs[selectedConfigIdx].id))
        {
            GenericPopup("Configuration removed successfully.").ShowModal();
        }
        else
        {
            LOG(ERROR) << "ChangeConfigurationDialog -> Failed to remove configuration.";
            GenericPopup("Failed to remove configuration.").ShowModal();
            return;
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << e.what();
        GenericPopup("Failed to open configuration database.").ShowModal();
        return;
    }

    PopulateConfigList();
    CheckIfConfigSelected();
    Update();
}

void ChangeConfigurationDialog::OnOK(wxCommandEvent &event)
{
    Global::SetCurrentConfig(configs[selectedConfigIdx]);
    EndModal(wxID_OK);
}

void ChangeConfigurationDialog::OnCancel(wxCommandEvent &event)
{
    EndModal(wxID_CANCEL);
}
