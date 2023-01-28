#include "GUI/GenericPopup.h"

#include "GUI/Misc.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(GenericPopup, wxDialog)
    EVT_BUTTON(wxID_OK, GenericPopup::OnOK)
    EVT_CHAR_HOOK(GenericPopup::CharHook)
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
    if (input)
    {
        ctrl.txtOutput->AppendText(Misc::stringToWx(*input));
        ctrl.txtOutput->Show();
    }
    if (output)
    {
        if (isPassword)
            ctrl.txtInputPass->Show();
        else
            ctrl.txtInputStd->Show();
    }
}

/******************************* EVENT HANDLERS ******************************/

void GenericPopup::OnOK(wxCommandEvent&)
{
    if (output)
    {
        if (isPassword)
            *output = Misc::wxToString(ctrl.txtInputPass->GetValue());
        else
            *output = Misc::wxToString(ctrl.txtInputStd->GetValue());
    }
    EndModal(wxID_OK);
}

void GenericPopup::CharHook(wxKeyEvent& event)
{
    switch (event.GetKeyCode())
    {
    case WXK_RETURN:
    {
        wxCommandEvent e;
        OnOK(e);
        break;
    }
    default:
        event.DoAllowNextEvent();
        break;
    }
}
