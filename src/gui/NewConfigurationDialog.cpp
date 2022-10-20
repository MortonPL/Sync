#include "NewConfigurationDialog.h"
#include <fmt/core.h>
#include "../lib/SSHConnector.h"
#include "../gui/GenericPopup.h"
#include "../utils/Logger.h"

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
}

void NewConfigurationDialog::CheckIfOK()
{
    auto txtConfigName = (wxTextCtrl*)(this->FindWindow("txtConfigName"));
    auto dirRootA = (wxDirPickerCtrl*)(this->FindWindow("dirRootA"));
    auto ddConfigType = (wxChoice*)(this->FindWindow("ddConfigType"));
    auto dirRootBLocal = (wxDirPickerCtrl*)(this->FindWindow("dirRootBLocal"));
    auto txtAddress = (wxTextCtrl*)(this->FindWindow("txtAddress"));
    auto txtUser = (wxTextCtrl*)(this->FindWindow("txtUser"));
    auto txtRootB = (wxTextCtrl*)(this->FindWindow("txtRootB"));

    bool isOK = !txtConfigName->GetValue().IsEmpty()
                && !dirRootA->GetPath().ToStdString().empty()
                && ( ddConfigType->GetSelection() == 0 ?
                    !dirRootBLocal->GetPath().ToStdString().empty()
                    :
                    (
                        !txtAddress->GetValue().IsEmpty()
                        && !txtUser->GetValue().IsEmpty()
                        && !txtRootB->GetValue().IsEmpty()
                    )
                );
    (wxButton*)(this->FindWindow("wxID_OK"))->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

void NewConfigurationDialog::OnOK(wxCommandEvent &event)
{
    auto ddConfigType = (wxChoice*)(this->FindWindow("ddConfigType"));
    auto txtAddress = (wxTextCtrl*)(this->FindWindow("txtAddress"));
    auto txtUser = (wxTextCtrl*)(this->FindWindow("txtUser"));
    auto txtRootB = (wxTextCtrl*)(this->FindWindow("txtRootB"));

    if (ddConfigType->GetSelection() == 1)
    {
        auto ssh = SSHConnector();
        GenericPopup popup;

        if (!ssh.BeginSession(txtAddress->GetValue().ToStdString()))
        {
            auto popup1 = GenericPopup("Failed to connect to given address.");
            popup1.ShowModal();
            return;
        }
        if (!ssh.AuthenticateServer())
        {
            popup = GenericPopup("Failed to authenticate server.");
            popup.ShowModal();
            return;
        }

        std::string password;
        popup = GenericPopup(
            fmt::format("Enter password for {}@{}:", txtUser->GetValue().ToStdString(), txtAddress->GetValue().ToStdString()),
            &password);
        popup.ShowModal();

        if (!ssh.AuthenticateUserPass(txtUser->GetValue().ToStdString(), password))
        {
            popup = GenericPopup("Failed to authenticate user. Check user credentials.");
            popup.ShowModal();
            return;
        }
        ssh.EndSession();

        popup = GenericPopup("Test connection successful.");
        popup.ShowModal();
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
    auto dirRootBLocal = (wxDirPickerCtrl*)(this->FindWindow("dirRootBLocal"));
    auto txtAddress = (wxTextCtrl*)(this->FindWindow("txtAddress"));
    auto txtUser = (wxTextCtrl*)(this->FindWindow("txtUser"));
    auto txtRootB = (wxTextCtrl*)(this->FindWindow("txtRootB"));

    switch (event.GetSelection())
    {
    case 0:
        dirRootBLocal->Enable();
        txtAddress->Disable();
        txtAddress->Clear();
        txtUser->Disable();
        txtUser->Clear();
        txtRootB->Disable();
        txtRootB->Clear();
        break;
    case 1:
        dirRootBLocal->Disable();
        dirRootBLocal->SetPath("");
        txtAddress->Enable();
        txtUser->Enable();
        txtRootB->Enable();
        break;
    default:
        break;
    }

    this->CheckIfOK();
}
