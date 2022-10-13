#include "NewConfigurationDialog.h"

wxBEGIN_EVENT_TABLE(NewConfigurationDialog, wxDialog)
    EVT_BUTTON(wxID_OK, NewConfigurationDialog::OnOK)
wxEND_EVENT_TABLE()

// ctor
NewConfigurationDialog::NewConfigurationDialog(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "NewConfigurationDialog");
}

/******************************* EVENT HANDLERS ******************************/

void NewConfigurationDialog::OnOK(wxCommandEvent &event)
{
    EndModal(wxID_OK);
}
