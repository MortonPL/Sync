#include "GUI/NewConfigurationDialog.h"

#include <fmt/core.h>
#include "GUI/GenericPopup.h"
#include "Lib/SSHConnector.h"
#include "Lib/DBConnector.h"
#include "Domain/Configuration.h"
#include "Utils/Logger.h"

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
}

void NewConfigurationDialog::CheckIfOK()
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

void NewConfigurationDialog::Update()
{
    this->CheckIfOK();
}

void NewConfigurationDialog::OnOK(wxCommandEvent &event)
{
    Configuration config;

    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    {
        config = Configuration(
            NOID,
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
            NOID,
            ctrl.txtConfigName->GetValue().ToStdString(),
            ctrl.dirRootA->GetPath().ToStdString(),
            ctrl.dirRootBLocal->GetPath().ToStdString()
        );
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
