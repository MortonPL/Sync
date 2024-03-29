#include "GUI/GenericPopup.h"

#include "GUI/Misc.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(GenericPopup, wxDialog)
    EVT_BUTTON(wxID_OK, GenericPopup::OnOK)
wxEND_EVENT_TABLE()

// ctor
GenericPopup::GenericPopup()
{
}

// ctor
GenericPopup::GenericPopup(std::string message, wxWindow* pParent, std::string* output, std::string* input, Flags flags)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "GenericPopup");

    ((wxStaticText*)(this->FindWindow("lblMessage")))->SetLabelText(Misc::stringToWx(message));

    ctrl = Controls
    {
        (wxTextCtrl*)(this->FindWindow("txtInputStd")),
        (wxTextCtrl*)(this->FindWindow("txtInputPass")),
        (wxTextCtrl*)(this->FindWindow("txtOutput")),
        (wxButton*)(this->FindWindow("wxID_OK")),
        (wxButton*)(this->FindWindow("wxID_CANCEL")),
    };

    this->input = input;
    this->output = output;
    this->isPassword = flags & Flags::Password;
    if (!(flags & Flags::Cancel))
        ctrl.btnCancel->Hide();
    if (this->input)
    {
        ctrl.txtOutput->AppendText(Misc::stringToWx(*input));
        ctrl.txtOutput->Show();
    }
    if (this->output)
    {
        if (this->isPassword)
            ctrl.txtInputPass->Show();
        else
            ctrl.txtInputStd->Show();
    }
}

/******************************* EVENT HANDLERS ******************************/

void GenericPopup::OnOK(wxCommandEvent &event)
{
    event.GetId(); // unused

    if (this->output)
    {
        if (this->isPassword)
            *(this->output) = Misc::wxToString(ctrl.txtInputPass->GetValue());
        else
            *(this->output) = Misc::wxToString(ctrl.txtInputStd->GetValue());
    }
    EndModal(wxID_OK);
}
