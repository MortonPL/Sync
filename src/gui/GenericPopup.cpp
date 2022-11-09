#include "GenericPopup.h"

#include "Logger.h"

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

    ctrl = Controls
    {
        (wxTextCtrl*)(this->FindWindow("txtInputStd")),
        (wxTextCtrl*)(this->FindWindow("txtInputPass")),
        (wxButton*)(this->FindWindow("wxID_OK")),
    };

    if (output)
    {
        if (isPassword)
            ctrl.txtInputPass->Show();
        else
            ctrl.txtInputStd->Show();
        this->output = output;
        this->isPassword = isPassword;
    }
}

/******************************* EVENT HANDLERS ******************************/

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
