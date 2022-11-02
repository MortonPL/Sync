#include <fmt/core.h>

#include "../headers/NewConfigurationDialog.h"
#include "../headers/SSHConnector.h"
#include "../headers/GenericPopup.h"
#include "../headers/Logger.h"

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
    #define DD_SSH 1
    if (ctrl.ddConfigType->GetSelection() == DD_SSH)
    #undef DD_SSH
    {
        auto ssh = SSHConnector();
        GenericPopup* popup;

        if (!ssh.BeginSession(ctrl.txtAddress->GetValue().ToStdString()))
        {
            popup = new GenericPopup("Failed to connect to given address.");
            popup->ShowModal();
            delete popup;
            return;
        }
        if (!ssh.AuthenticateServer())
        {
            popup = new GenericPopup("Failed to authenticate server.");
            popup->ShowModal();
            delete popup;
            ssh.EndSession();
            return;
        }

        std::string password;
        popup = new GenericPopup(
            fmt::format("Enter password for {}@{}:", ctrl.txtUser->GetValue().ToStdString(), ctrl.txtAddress->GetValue().ToStdString()),
            &password, true);
        popup->ShowModal();
        delete popup;

        if (!ssh.AuthenticateUserPass(ctrl.txtUser->GetValue().ToStdString(), password))
        {
            popup = new GenericPopup("Failed to authenticate user. Check user credentials.");
            popup->ShowModal();
            delete popup;
            ssh.EndSession();
            return;
        }

        if (!ssh.ExecuteCD(ctrl.txtRootB->GetValue().ToStdString()))
        {
            popup = new GenericPopup("Failed to find remote directory. Check name or permissions.");
            popup->ShowModal();
            delete popup;
            ssh.EndSession();
            return;
        }

        ssh.EndSession();
        popup = new GenericPopup("Test connection successful.");
        popup->ShowModal();
        delete popup;
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
    switch (event.GetSelection())
    {
    case 0:
        ctrl.dirRootBLocal->Enable();
        ctrl.txtAddress->Disable();
        ctrl.txtAddress->Clear();
        ctrl.txtUser->Disable();
        ctrl.txtUser->Clear();
        ctrl.txtRootB->Disable();
        ctrl.txtRootB->Clear();
        break;
    case 1:
        ctrl.dirRootBLocal->Disable();
        ctrl.dirRootBLocal->SetPath("");
        ctrl.txtAddress->Enable();
        ctrl.txtUser->Enable();
        ctrl.txtRootB->Enable();
        break;
    default:
        break;
    }

    this->CheckIfOK();
}
