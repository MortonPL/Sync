#include "GUI/EditConfigurationDialog.h"

#include "GUI/GenericPopup.h"
#include "Lib/DBConnector.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(EditConfigurationDialog, wxDialog)
    EVT_BUTTON(wxID_OK, EditConfigurationDialog::OnOK)
    EVT_TEXT(XRCID("txtConfigName"), EditConfigurationDialog::OnAnyChange)
    EVT_DIRPICKER_CHANGED(XRCID("dirRootA"), EditConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtAddressB"), EditConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtUserB"), EditConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtRootB"), EditConfigurationDialog::OnAnyChange)
wxEND_EVENT_TABLE()

// ctor
EditConfigurationDialog::EditConfigurationDialog(Configuration& oldConfig, wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "NewConfigurationDialog");

    ctrl = Controls
    {
        (wxTextCtrl*)(FindWindow("txtConfigName")),
        (wxDirPickerCtrl*)(FindWindow("dirRootA")),
        (wxTextCtrl*)(FindWindow("txtAddressB")),
        (wxTextCtrl*)(FindWindow("txtUserB")),
        (wxTextCtrl*)(FindWindow("txtRootB")),
        (wxButton*)(FindWindow("wxID_OK")),
        (wxButton*)(FindWindow("wxID_CANCEL")),
    };

    this->oldConfig = oldConfig;

    //fill in controls
    ctrl.txtConfigName->AppendText(oldConfig.name);
    ctrl.dirRootA->SetPath(oldConfig.pathA);
    ctrl.txtRootB->AppendText(oldConfig.pathB);
    ctrl.txtAddressB->AppendText(oldConfig.pathBaddress);
    ctrl.txtUserB->AppendText(oldConfig.pathBuser);

    ctrl.btnOK->SetLabel("Edit");
    ctrl.btnOK->Disable();
}

void EditConfigurationDialog::CheckIfOK()
{
    bool isOK = !ctrl.txtConfigName->GetValue().IsEmpty()
                && !ctrl.dirRootA->GetPath().ToStdString().empty()
                && !ctrl.txtRootB->GetValue().IsEmpty()
                && !ctrl.txtAddressB->GetValue().IsEmpty()
                && !ctrl.txtUserB->GetValue().IsEmpty();
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

void EditConfigurationDialog::OnOK(wxCommandEvent &event)
{
    std::string pathA = ctrl.dirRootA->GetPath().ToStdString();
    std::string pathB = ctrl.txtRootB->GetValue().ToStdString();
    if (pathA.empty() || pathB.empty())
    {
        GenericPopup("Root paths cannot be empty!").ShowModal();
        return;
    }

    Configuration config;
    config = Configuration(
        oldConfig.id,
        ctrl.txtConfigName->GetValue().ToStdString(),
        oldConfig.uuid,
        oldConfig.timestamp,
        Utils::CorrectDirPath(pathA),
        Utils::CorrectDirPath(pathB),
        ctrl.txtAddressB->GetValue().ToStdString(),
        ctrl.txtUserB->GetValue().ToStdString(),
        0
    );

    if (config == oldConfig)
    {
        EndModal(wxID_CANCEL);
        return;
    }

    try
    {
        DBConnector db(DBConnector::GetMainFileName(), SQLite::OPEN_READWRITE);
        db.UpdateConfig(config);
    }
    catch(const std::exception& e)
    {
        GenericPopup("Failed to open configuration database.").ShowModal();
        return;
    }
    GenericPopup("Successfully edited the configuration.").ShowModal();

    EndModal(wxID_OK);
}

void EditConfigurationDialog::OnAnyChange(wxCommandEvent &event)
{
    this->CheckIfOK();
}

void EditConfigurationDialog::OnAnyChange(wxFileDirPickerEvent &event)
{
    this->CheckIfOK();
}
