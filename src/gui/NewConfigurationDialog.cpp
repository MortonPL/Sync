#include <fmt/core.h>

#include "NewConfigurationDialog.h"
#include "SSHConnector.h"
#include "GenericPopup.h"
#include "Logger.h"
#include "Domain/Configuration.h"
#include "DBConnector.h"

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

void NewConfigurationDialog::OnOK(wxCommandEvent &event)
{
    GenericPopup* popup;
    Configuration config;

    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    {
        auto ssh = SSHConnector();

        if (!ssh.BeginSession(ctrl.txtAddress->GetValue().ToStdString()))
        {
            popup = new GenericPopup("Failed to connect to given address.");
            popup->ShowModal();
            popup->Destroy();
            return;
        }
        if (!ssh.AuthenticateServer())
        {
            popup = new GenericPopup("Failed to authenticate server.");
            popup->ShowModal();
            popup->Destroy();
            ssh.EndSession();
            return;
        }

        std::string password;
        popup = new GenericPopup(
            fmt::format("Enter password for {}@{}:", ctrl.txtUser->GetValue().ToStdString(), ctrl.txtAddress->GetValue().ToStdString()),
            &password, true);
        popup->ShowModal();
        popup->Destroy();

        if (!ssh.AuthenticateUserPass(ctrl.txtUser->GetValue().ToStdString(), password))
        {
            popup = new GenericPopup("Failed to authenticate user. Check user credentials.");
            popup->ShowModal();
            popup->Destroy();
            ssh.EndSession();
            return;
        }

        if (!ssh.ExecuteCD(ctrl.txtRootB->GetValue().ToStdString()))
        {
            popup = new GenericPopup("Failed to find remote directory. Check name or permissions.");
            popup->ShowModal();
            popup->Destroy();
            ssh.EndSession();
            return;
        }

        ssh.EndSession();
        popup = new GenericPopup("Test connection successful.");
        popup->ShowModal();
        popup->Destroy();

        config = Configuration(
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
            ctrl.txtConfigName->GetValue().ToStdString(),
            ctrl.dirRootA->GetPath().ToStdString(),
            ctrl.dirRootBLocal->GetPath().ToStdString()
        );
    }

    auto db = DBConnector();
    if (db.Open(SQLite::OPEN_READWRITE))
    {
        db.InsertConfig(config);
    }
    else
    {
        popup = new GenericPopup("Failed to open configuration database.");
        popup->ShowModal();
        popup->Destroy();
    }

    EndModal(wxID_OK);
}

void NewConfigurationDialog::OnAnyChange(wxCommandEvent &event)
{
    this->CheckIfOK();
}

void NewConfigurationDialog::OnAnyChange(wxFileDirPickerEvent &event)
{
    this->CheckIfOK();
}

void NewConfigurationDialog::OnConfigTypeChange(wxCommandEvent &event)
{
    if (event.GetSelection() == DD_SSH)
    {
        ctrl.dirRootBLocal->Disable();
        ctrl.dirRootBLocal->SetPath("");
        ctrl.txtAddress->Enable();
        ctrl.txtUser->Enable();
        ctrl.txtRootB->Enable();
    }
    else
    {
        ctrl.dirRootBLocal->Enable();
        ctrl.txtAddress->Disable();
        ctrl.txtAddress->Clear();
        ctrl.txtUser->Disable();
        ctrl.txtUser->Clear();
        ctrl.txtRootB->Disable();
        ctrl.txtRootB->Clear();
    }

    this->CheckIfOK();
}

#undef DD_SSH
