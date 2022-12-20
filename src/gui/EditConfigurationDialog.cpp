#include "GUI/EditConfigurationDialog.h"

#include "GUI/GenericPopup.h"
#include "GUI/SSHConnectorWrap.h"
#include "Lib/DBConnector.h"
#include "Lib/SSHConnector.h"
#include "Lib/Global.h"
#include "Utils.h"

#define DD_SSH 1

wxBEGIN_EVENT_TABLE(EditConfigurationDialog, wxDialog)
    EVT_BUTTON(wxID_OK, EditConfigurationDialog::OnOK)
    EVT_TEXT(XRCID("txtConfigName"), EditConfigurationDialog::OnAnyChange)
    EVT_DIRPICKER_CHANGED(XRCID("dirRootA"), EditConfigurationDialog::OnAnyChange)
    EVT_CHOICE(XRCID("ddConfigType"), EditConfigurationDialog::OnConfigTypeChange)
    EVT_DIRPICKER_CHANGED(XRCID("dirRootBLocal"), EditConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtAddress"), EditConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtUser"), EditConfigurationDialog::OnAnyChange)
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
        (wxChoice*)(FindWindow("ddConfigType")),
        (wxDirPickerCtrl*)(FindWindow("dirRootBLocal")),
        (wxTextCtrl*)(FindWindow("txtAddressB")),
        (wxTextCtrl*)(FindWindow("txtUserB")),
        (wxTextCtrl*)(FindWindow("txtRootB")),
        (wxTextCtrl*)(FindWindow("txtAddressA")),
        (wxTextCtrl*)(FindWindow("txtUserA")),
        (wxButton*)(FindWindow("wxID_OK")),
        (wxButton*)(FindWindow("wxID_CANCEL")),
    };

    this->oldConfig = oldConfig;

    //fill in controls
    ctrl.txtConfigName->AppendText(oldConfig.name);
    ctrl.dirRootA->SetPath(oldConfig.pathA);
    ctrl.ddConfigType->SetSelection(oldConfig.isRemote ? 1 : 0);
    if (oldConfig.isRemote)
    {
        ctrl.txtRootB->AppendText(oldConfig.pathB);
        ctrl.txtAddressB->AppendText(oldConfig.pathBaddress);
        ctrl.txtUserB->AppendText(oldConfig.pathBuser);
        ctrl.txtAddressA->AppendText(oldConfig.pathAaddress);
        ctrl.txtUserA->AppendText(oldConfig.pathAuser);
        EnableRemote();
    }
    else
    {
        ctrl.dirRootBLocal->SetPath(oldConfig.pathB);
        DisableRemote();
    }
    ctrl.btnOK->SetLabel("Edit");
}

void EditConfigurationDialog::CheckIfOK()
{
    bool isOK = !ctrl.txtConfigName->GetValue().IsEmpty()
                && !ctrl.dirRootA->GetPath().ToStdString().empty()
                && (ctrl.ddConfigType->GetSelection() == 0 ?
                    !ctrl.dirRootBLocal->GetPath().ToStdString().empty()
                    :
                    (
                        !ctrl.txtRootB->GetValue().IsEmpty()
                        && !ctrl.txtAddressB->GetValue().IsEmpty()
                        && !ctrl.txtUserB->GetValue().IsEmpty()
                        && !ctrl.txtAddressA->GetValue().IsEmpty()
                        && !ctrl.txtUserA->GetValue().IsEmpty()
                    )
                );
    ctrl.btnOK->Enable(isOK);
}

void EditConfigurationDialog::DisableRemote()
{
    ctrl.dirRootBLocal->Enable();
    ctrl.txtRootB->Disable();
    ctrl.txtAddressB->Disable();
    ctrl.txtUserB->Disable();
    ctrl.txtAddressA->Disable();
    ctrl.txtUserA->Disable();
}

void EditConfigurationDialog::EnableRemote()
{
    ctrl.dirRootBLocal->Disable();
    ctrl.txtRootB->Enable();
    ctrl.txtAddressB->Enable();
    ctrl.txtUserB->Enable();
    ctrl.txtAddressA->Enable();
    ctrl.txtUserA->Enable();
}

/******************************* EVENT HANDLERS ******************************/

void EditConfigurationDialog::Update()
{
    CheckIfOK();
}

void EditConfigurationDialog::OnOK(wxCommandEvent &event)
{
    Configuration config;

    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    {
        config = Configuration(
            oldConfig.id,
            ctrl.txtConfigName->GetValue().ToStdString(),
            oldConfig.uuid,
            Utils::CorrectDirPath(ctrl.dirRootA->GetPath().ToStdString()),
            ctrl.txtAddressA->GetValue().ToStdString(),
            ctrl.txtUserA->GetValue().ToStdString(),
            Utils::CorrectDirPath(ctrl.txtRootB->GetValue().ToStdString()),
            ctrl.txtAddressB->GetValue().ToStdString(),
            ctrl.txtUserB->GetValue().ToStdString()
        );
    }
    else
    {
        config = Configuration(
            oldConfig.id,
            ctrl.txtConfigName->GetValue().ToStdString(),
            oldConfig.uuid,
            Utils::CorrectDirPath(ctrl.dirRootA->GetPath().ToStdString()),
            Utils::CorrectDirPath(ctrl.dirRootBLocal->GetPath().ToStdString())
        );
    }

    if (config == oldConfig)
    {
        EndModal(wxID_CANCEL);
    }

    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    {
        auto ssh = SSHConnector();
        if (!SSHConnectorWrap::Connect(ssh, ctrl.txtAddressB->GetValue().ToStdString(), ctrl.txtUserB->GetValue().ToStdString()))
            return;
        uuid_copy(Global::lastUsedCreds.uuid, oldConfig.uuid);

        int rc = ssh.CallCLITest(ctrl.txtRootB->GetValue().ToStdString());
        if (rc != 0)
        {
            if (rc == 1)
                GenericPopup("Failed to call Sync on the remote.").ShowModal();
            else if (rc == 2)
                GenericPopup("Failed to find remote directory. Check path or permissions.").ShowModal();
            ssh.EndSession();
            return;
        }

        ssh.EndSession();
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
    Update();
}

void EditConfigurationDialog::OnAnyChange(wxFileDirPickerEvent &event)
{
    Update();
}

void EditConfigurationDialog::OnConfigTypeChange(wxCommandEvent &event)
{
    if (event.GetSelection() == DD_SSH)
        EnableRemote();
    else
        DisableRemote();

    Update();
}

#undef DD_SSH
