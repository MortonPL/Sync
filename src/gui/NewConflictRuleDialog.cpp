#include "GUI/NewConflictRuleDialog.h"

#include "GUI/GenericPopup.h"
#include "Lib/Global.h"
#include "Lib/DBConnector.h"
#include "Domain/ConflictRule.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(NewConflictRuleDialog, wxDialog)
    EVT_BUTTON(wxID_OK, NewConflictRuleDialog::OnOK)
    EVT_TEXT(XRCID("txtName"), NewConflictRuleDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtRule"), NewConflictRuleDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtCommand"), NewConflictRuleDialog::OnAnyChange)
wxEND_EVENT_TABLE()

// ctor
NewConflictRuleDialog::NewConflictRuleDialog(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "NewConflictRuleDialog");

    ctrl = Controls
    {
        (wxTextCtrl*)(FindWindow("txtName")),
        (wxTextCtrl*)(FindWindow("txtRule")),
        (wxTextCtrl*)(FindWindow("txtCommand")),
        (wxButton*)(FindWindow("wxID_OK")),
        (wxButton*)(FindWindow("wxID_CANCEL")),
    };
}

void NewConflictRuleDialog::CheckIfOK()
{
    bool isOK = !ctrl.txtName->GetValue().utf8_string().empty()
                && !ctrl.txtRule->GetValue().utf8_string().empty()
                && !ctrl.txtCommand->GetValue().utf8_string().empty();
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

void NewConflictRuleDialog::OnOK(wxCommandEvent &event)
{
    event.GetId(); //unused
    auto rule = ConflictRule(
        ctrl.txtName->GetValue().utf8_string(),
        ctrl.txtRule->GetValue().utf8_string(),
        ctrl.txtCommand->GetValue().utf8_string()
    );

    if (rule.badRule)
    {
        GenericPopup("Entered rule is not a valid pseudo-regex rule!").ShowModal();
        return;
    }

    try
    {
        DBConnector db(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid), SQLite::OPEN_READWRITE);
        if (db.InsertConflictRule(rule))
        {
            GenericPopup("Successfully created a new conflict rule.").ShowModal();
        }
        else
        {
            GenericPopup("Failed to create a new conflict rule.").ShowModal();
        }
    }
    catch(const std::exception& e)
    {
        GenericPopup("Failed to open conflict rule database.").ShowModal();
        return;
    }

    EndModal(wxID_OK);
}

void NewConflictRuleDialog::OnAnyChange(wxCommandEvent &event)
{
    event.GetId(); //unused
    this->CheckIfOK();
}
