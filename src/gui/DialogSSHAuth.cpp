#include "DialogSSHAuth.h"

wxBEGIN_EVENT_TABLE(DialogSSHAuth, wxDialog)
    EVT_BUTTON(wxID_OK, DialogSSHAuth::OnOK)
wxEND_EVENT_TABLE()

// ctor
DialogSSHAuth::DialogSSHAuth(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "MyDialog1");

    pUser = XRCCTRL(*this, "txtLogin", wxTextCtrl);
    pPass = XRCCTRL(*this, "txtPass", wxTextCtrl);
}

/******************************* EVENT HANDLERS ******************************/

void DialogSSHAuth::OnOK(wxCommandEvent &event)
{
    userValue = pUser->GetValue();
    passValue = pPass->GetValue();
    EndModal(wxID_OK);
}
