#include "GUI/NewConfigurationDialog.h"

#include <uuid/uuid.h>
#include "GUI/GenericPopup.h"
#include "Lib/SSHConnector.h"
#include "Lib/DBConnector.h"
#include "Domain/Configuration.h"
#include "Utils.h"

#define DD_SSH 1

wxBEGIN_EVENT_TABLE(NewConfigurationDialog, wxDialog)
    EVT_BUTTON(wxID_OK, NewConfigurationDialog::OnOK)
    EVT_TEXT(XRCID("txtConfigName"), NewConfigurationDialog::OnAnyChange)
    EVT_DIRPICKER_CHANGED(XRCID("dirRootA"), NewConfigurationDialog::OnAnyChange)
    EVT_CHOICE(XRCID("ddConfigType"), NewConfigurationDialog::OnConfigTypeChange)
    EVT_DIRPICKER_CHANGED(XRCID("dirRootBLocal"), NewConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtAddress"), NewConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtUser"), NewConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtRootB"), NewConfigurationDialog::OnAnyChange)
wxEND_EVENT_TABLE()

// ctor
NewConfigurationDialog::NewConfigurationDialog(wxWindow* pParent)
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
}

void NewConfigurationDialog::CheckIfOK()
{
    // TODO a bug here requires you to edit config name?
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

void NewConfigurationDialog::DisableRemote()
{
    ctrl.dirRootBLocal->Enable();
    ctrl.txtRootB->Disable();
    ctrl.txtAddressB->Disable();
    ctrl.txtUserB->Disable();
    ctrl.txtAddressA->Disable();
    ctrl.txtUserA->Disable();
}

void NewConfigurationDialog::EnableRemote()
{
    ctrl.dirRootBLocal->Disable();
    ctrl.txtRootB->Enable();
    ctrl.txtAddressB->Enable();
    ctrl.txtUserB->Enable();
    ctrl.txtAddressA->Enable();
    ctrl.txtUserA->Enable();
}

/******************************* EVENT HANDLERS ******************************/

void NewConfigurationDialog::Update()
{
    this->CheckIfOK();
}

void NewConfigurationDialog::OnOK(wxCommandEvent &event)
{
    Configuration config;
    uuid_t uuid;
    uuid_generate(uuid);

    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    {
        config = Configuration(
            NOID,
            ctrl.txtConfigName->GetValue().ToStdString(),
            uuid,
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
            NOID,
            ctrl.txtConfigName->GetValue().ToStdString(),
            uuid,
            Utils::CorrectDirPath(ctrl.dirRootA->GetPath().ToStdString()),
            Utils::CorrectDirPath(ctrl.dirRootBLocal->GetPath().ToStdString())
        );
    }

    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    {
        auto ssh = SSHConnector();

        if (!ssh.BeginSession(ctrl.txtAddressB->GetValue().ToStdString()))
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
            fmt::format("Enter password for {}@{}:", ctrl.txtUserB->GetValue().ToStdString(), ctrl.txtAddressB->GetValue().ToStdString()),
            NULL, &password, true).ShowModal();

        if (!ssh.AuthenticateUserPass(ctrl.txtUserB->GetValue().ToStdString(), password))
        {
            GenericPopup("Failed to authenticate user. Check user credentials.").ShowModal();
            ssh.EndSession();
            return;
        }

        if (!ssh.CallCLI())
        {
            GenericPopup("Failed to call Sync on the remote.").ShowModal();
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
    }

    try
    {
        DBConnector db(DBConnector::GetMainFileName(), SQLite::OPEN_READWRITE);
        db.InsertConfig(config);
    }
    catch(const std::exception& e)
    {
        GenericPopup("Failed to open configuration database.").ShowModal();
        return;
    }
    GenericPopup("Successfully created a new configuration.").ShowModal();

    EndModal(wxID_OK);
}

void NewConfigurationDialog::OnAnyChange(wxCommandEvent &event)
{
    Update();
}

void NewConfigurationDialog::OnAnyChange(wxFileDirPickerEvent &event)
{
    Update();
}

void NewConfigurationDialog::OnConfigTypeChange(wxCommandEvent &event)
{
    if (event.GetSelection() == DD_SSH)
    {
        EnableRemote();
    }
    else
    {
        DisableRemote();
    }

    Update();
}

#undef DD_SSH
