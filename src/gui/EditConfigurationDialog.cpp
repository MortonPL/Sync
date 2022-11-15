#include <fmt/core.h>

#include "EditConfigurationDialog.h"
#include "GenericPopup.h"
#include "DBConnector.h"
#include "SSHConnector.h"

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
        (wxTextCtrl*)(this->FindWindow("txtConfigName")),
        (wxDirPickerCtrl*)(this->FindWindow("dirRootA")),
        (wxChoice*)(this->FindWindow("ddConfigType")),
        (wxDirPickerCtrl*)(this->FindWindow("dirRootBLocal")),
        (wxTextCtrl*)(this->FindWindow("txtAddress")),
        (wxTextCtrl*)(this->FindWindow("txtUser")),
        (wxTextCtrl*)(this->FindWindow("txtRootB")),
        (wxButton*)(this->FindWindow("wxID_OK")),
        (wxButton*)(this->FindWindow("wxID_CANCEL")),
    };

    this->oldConfig = oldConfig;

    //fill in controls
    ctrl.txtConfigName->AppendText(this->oldConfig.name);
    ctrl.dirRootA->SetPath(this->oldConfig.pathA);
    ctrl.ddConfigType->SetSelection((int)(this->oldConfig.isRemote));
    if (this->oldConfig.isRemote)
    {
        ctrl.txtAddress->AppendText(this->oldConfig.remoteAddress);
        ctrl.txtUser->AppendText(this->oldConfig.remoteUser);
        ctrl.txtRootB->AppendText(this->oldConfig.pathB);
    }
    else
    {
        ctrl.dirRootBLocal->SetPath(this->oldConfig.pathB);
    }
}

void EditConfigurationDialog::CheckIfOK()
{
    bool isOK = !ctrl.txtConfigName->GetValue().IsEmpty()
                && !ctrl.dirRootA->GetPath().ToStdString().empty()
                && (ctrl.ddConfigType->GetSelection() == 0 ?
                    !ctrl.dirRootBLocal->GetPath().ToStdString().empty()
                    :
                    (
                        !ctrl.txtAddress->GetValue().IsEmpty()
                        && !ctrl.txtUser->GetValue().IsEmpty()
                        && !ctrl.txtRootB->GetValue().IsEmpty()
                    )
                );
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

void EditConfigurationDialog::Update()
{
    this->CheckIfOK();
}

void EditConfigurationDialog::OnOK(wxCommandEvent &event)
{
    Configuration config;

    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    {
        config = Configuration(
            this->oldConfig.id,
            ctrl.txtConfigName->GetValue().ToStdString(),
            ctrl.dirRootA->GetPath().ToStdString(),
            ctrl.txtRootB->GetValue().ToStdString(),
            ctrl.txtAddress->GetValue().ToStdString(),
            ctrl.txtUser->GetValue().ToStdString()
        );
    }
    else
    {
        config = Configuration(
            this->oldConfig.id,
            ctrl.txtConfigName->GetValue().ToStdString(),
            ctrl.dirRootA->GetPath().ToStdString(),
            ctrl.dirRootBLocal->GetPath().ToStdString()
        );
    }

    if (config == this->oldConfig)
    {
        EndModal(wxID_CANCEL);
    }

    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    {
        auto ssh = SSHConnector();

        if (!ssh.BeginSession(ctrl.txtAddress->GetValue().ToStdString()))
        {
            GenericPopup("Failed to connect to given address.").ShowModal();
            return;
        }
        if (!ssh.AuthenticateServer())
        {
            GenericPopup("Failed to authenticate server.").ShowModal();
            ssh.EndSession();
            return;
        }

        std::string password;
        GenericPopup(
            fmt::format("Enter password for {}@{}:", ctrl.txtUser->GetValue().ToStdString(), ctrl.txtAddress->GetValue().ToStdString()),
            NULL, &password, true).ShowModal();

        // TODO support key auth
        if (!ssh.AuthenticateUserPass(ctrl.txtUser->GetValue().ToStdString(), password))
        {
            GenericPopup("Failed to authenticate user. Check user credentials.").ShowModal();
            ssh.EndSession();
            return;
        }

        if (!ssh.ExecuteCD(ctrl.txtRootB->GetValue().ToStdString()))
        {
            GenericPopup("Failed to find remote directory. Check name or permissions.").ShowModal();
            ssh.EndSession();
            return;
        }

        ssh.EndSession();
        GenericPopup("Test connection successful.").ShowModal();
    }

    try
    {
        DBConnector db(SQLite::OPEN_READWRITE);
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
    {
        ctrl.dirRootBLocal->Disable();
        ctrl.txtAddress->Enable();
        ctrl.txtUser->Enable();
        ctrl.txtRootB->Enable();
    }
    else
    {
        ctrl.dirRootBLocal->Enable();
        ctrl.txtAddress->Disable();
        ctrl.txtUser->Disable();
        ctrl.txtRootB->Disable();
    }

    Update();
}

#undef DD_SSH
