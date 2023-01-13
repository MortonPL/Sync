#include "GUI/ChangeConfigurationDialog.h"

#include <cstdio>
#include "GUI/Misc.h"
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
    try
    {
        DBConnector db(DBConnector::GetMainFileName());
        db.SelectAllConfigs(this->configs);
        auto arrs = wxArrayString();
        for(auto config: this->configs)
            arrs.Add(Misc::stringToWx(config.name));
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
    *ctrl.txtDetails << "\tName: " << Misc::stringToWx(config.name) << "\n";
    *ctrl.txtDetails << "\tPath A: " << Misc::stringToWx(config.pathA) << "\n";
    *ctrl.txtDetails << "\tPath B: " << Misc::stringToWx(config.pathB) << "\n";
    *ctrl.txtDetails << "\tLast config edit: " << Utils::TimestampToString(config.timestamp) << "\n";
    *ctrl.txtDetails << "\tPath B Address/Hostname: " << Misc::stringToWx(config.pathBaddress) << "\n";
    *ctrl.txtDetails << "\tPath B User: " << Misc::stringToWx(config.pathBuser) << "\n";
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

void ChangeConfigurationDialog::OnListBoxChange(wxCommandEvent &event)
{
    event.GetId(); //unused
    CheckIfConfigSelected();
}

void ChangeConfigurationDialog::OnNewConfig(wxCommandEvent &event)
{
    event.GetId(); //unused
    NewConfigurationDialog().ShowModal();
    PopulateConfigList();
}

void ChangeConfigurationDialog::OnEditConfig(wxCommandEvent &event)
{
    event.GetId(); //unused
    if (EditConfigurationDialog(configs[selectedConfigIdx]).ShowModal() == wxID_OK)
    {
        PopulateConfigList();
        PopulateConfigDetails();
    }
}

void ChangeConfigurationDialog::OnDeleteConfig(wxCommandEvent &event)
{
    event.GetId(); //unused
    try
    {
        DBConnector db(DBConnector::GetMainFileName(), SQLite::OPEN_READWRITE);
        if (db.DeleteConfig(configs[selectedConfigIdx].id))
        {
            remove(Utils::UUIDToDBPath(configs[selectedConfigIdx].uuid).c_str());
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
}

void ChangeConfigurationDialog::OnOK(wxCommandEvent &event)
{
    event.GetId(); //unused
    Global::CurrentConfig(configs[selectedConfigIdx]);
    EndModal(wxID_OK);
}

void ChangeConfigurationDialog::OnCancel(wxCommandEvent &event)
{
    event.GetId(); //unused
    EndModal(wxID_CANCEL);
}
