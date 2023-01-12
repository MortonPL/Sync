#include "GUI/EditConflictRuleDialog.h"

#include "GUI/GenericPopup.h"
#include "Lib/Global.h"
#include "Lib/DBConnector.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(EditConflictRuleDialog, wxDialog)
    EVT_BUTTON(wxID_OK, EditConflictRuleDialog::OnOK)
    EVT_TEXT(XRCID("txtName"), EditConflictRuleDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtRule"), EditConflictRuleDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtCommand"), EditConflictRuleDialog::OnAnyChange)
wxEND_EVENT_TABLE()

// ctor
EditConflictRuleDialog::EditConflictRuleDialog(ConflictRule& oldRule, wxWindow* pParent)
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

    this->oldRule = oldRule;

    //fill in controls
    ctrl.txtName->AppendText(wxString::FromUTF8(oldRule.name));
    ctrl.txtRule->AppendText(wxString::FromUTF8(oldRule.rule));
    ctrl.txtCommand->AppendText(wxString::FromUTF8(oldRule.command));

    ctrl.btnOK->SetLabel("Edit");
    ctrl.btnOK->Disable();
}

void EditConflictRuleDialog::CheckIfOK()
{
    bool isOK = !ctrl.txtName->GetValue().utf8_string().empty()
                && !ctrl.txtRule->GetValue().utf8_string().empty()
                && !ctrl.txtCommand->GetValue().utf8_string().empty();
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

void EditConflictRuleDialog::OnOK(wxCommandEvent &event)
{
    event.GetId(); //unused
    auto rule = ConflictRule(
        oldRule.id,
        oldRule.order,
        ctrl.txtName->GetValue().utf8_string(),
        ctrl.txtRule->GetValue().utf8_string(),
        ctrl.txtCommand->GetValue().utf8_string()
    );

    if (rule.badRule)
    {
        GenericPopup("Entered rule is not a valid pseudo-regex rule!").ShowModal();
        return;
    }

    if (rule == oldRule)
    {
        EndModal(wxID_CANCEL);
        return;
    }

    try
    {
        DBConnector db(Utils::UUIDToDBPath(Global::CurrentConfig().uuid), SQLite::OPEN_READWRITE);
        if (db.UpdateConflictRule(rule))
        {
            GenericPopup("Successfully updated the conflict rule.").ShowModal();
        }
        else
        {
            GenericPopup("Failed to update the conflict rule.").ShowModal();
        }
    }
    catch(const std::exception& e)
    {
        GenericPopup("Failed to open conflict rule database.").ShowModal();
        return;
    }

    EndModal(wxID_OK);
}

void EditConflictRuleDialog::OnAnyChange(wxCommandEvent &event)
{
    event.GetId(); //unused
    this->CheckIfOK();
}
