#include "../headers/SSHAuthDialog.h"

wxBEGIN_EVENT_TABLE(SSHAuthDialog, wxDialog)
    EVT_BUTTON(wxID_OK, SSHAuthDialog::OnOK)
wxEND_EVENT_TABLE()

// ctor
SSHAuthDialog::SSHAuthDialog(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "SSHAuthDialog");

    pUser = XRCCTRL(*this, "txtLogin", wxTextCtrl);
    pPass = XRCCTRL(*this, "txtPass", wxTextCtrl);
}

/******************************* EVENT HANDLERS ******************************/

void SSHAuthDialog::OnOK(wxCommandEvent &event)
{
    userValue = pUser->GetValue();
    passValue = pPass->GetValue();
    EndModal(wxID_OK);
}
