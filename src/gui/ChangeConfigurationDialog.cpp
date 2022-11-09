#include "ChangeConfigurationDialog.h"

wxBEGIN_EVENT_TABLE(ChangeConfigurationDialog, wxDialog)
    EVT_BUTTON(wxID_OK, ChangeConfigurationDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ChangeConfigurationDialog::OnCancel)
wxEND_EVENT_TABLE()

// ctor
ChangeConfigurationDialog::ChangeConfigurationDialog(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "ChangeConfigurationDialog");
}

/******************************* EVENT HANDLERS ******************************/

void ChangeConfigurationDialog::OnOK(wxCommandEvent &event)
{
    EndModal(wxID_OK);
}

void ChangeConfigurationDialog::OnCancel(wxCommandEvent &event)
{
    EndModal(wxID_CANCEL);
}
