#include "GUI/GenericPopup.h"

#include "Utils.h"

wxBEGIN_EVENT_TABLE(GenericPopup, wxDialog)
    EVT_BUTTON(wxID_OK, GenericPopup::OnOK)
wxEND_EVENT_TABLE()

// ctor
GenericPopup::GenericPopup()
{
}

// ctor
GenericPopup::GenericPopup(std::string message, wxWindow* pParent, std::string* output, bool isPassword)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "GenericPopup");

    auto lblMessage = (wxStaticText*)(this->FindWindow("lblMessage"));
    lblMessage->SetLabelText(message);

    ctrl = Controls
    {
        (wxTextCtrl*)(this->FindWindow("txtInputStd")),
        (wxTextCtrl*)(this->FindWindow("txtInputPass")),
        (wxButton*)(this->FindWindow("wxID_OK")),
    };

    this->output = output;
    this->isPassword = isPassword;
    if (this->output)
    {
        if (this->isPassword)
            ctrl.txtInputPass->Show();
        else
            ctrl.txtInputStd->Show();
    }
}

/******************************* EVENT HANDLERS ******************************/

void GenericPopup::Update()
{
}

void GenericPopup::OnOK(wxCommandEvent &event)
{
    if (this->output)
    {
        if (this->isPassword)
            *(this->output) = ctrl.txtInputPass->GetValue().ToStdString();
        else
            *(this->output) = ctrl.txtInputStd->GetValue().ToStdString();
    }
    EndModal(wxID_OK);
}
