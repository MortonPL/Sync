#include "../headers/GenericPopup.h"

wxBEGIN_EVENT_TABLE(GenericPopup, wxDialog)
    EVT_BUTTON(wxID_OK, GenericPopup::OnOK)
wxEND_EVENT_TABLE()

// ctor
GenericPopup::GenericPopup()
{
}

// ctor
GenericPopup::GenericPopup(std::string message, std::string* output, bool isPassword, wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "GenericPopup");

    auto lblMessage = (wxStaticText*)(this->FindWindow("lblMessage"));
    lblMessage->SetLabelText(message);
    if (output)
    {
        auto txtInput1 = (wxTextCtrl*)(this->FindWindow("txtInput1"));
        txtInput1->Show();
        this->output = output;
        if (isPassword)
        {
            txtInput1->SetDefaultStyle(wxTextAttr(wxTE_PASSWORD));
        }
    }
}

GenericPopup& GenericPopup::operator=(const GenericPopup& other)
{
    if (&other != this)
    {
        this->output = other.output;
    }
    return *this;
}

/******************************* EVENT HANDLERS ******************************/

void GenericPopup::OnOK(wxCommandEvent &event)
{
    *(this->output) = ((wxTextCtrl*)(this->FindWindow("txtInput1")))->GetValue().ToStdString();
    EndModal(wxID_OK);
}
