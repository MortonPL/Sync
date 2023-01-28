#include "GUI/NewConflictRuleDialog.h"

#include "GUI/Misc.h"
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
    bool isOK = !Misc::wxToString(ctrl.txtName->GetValue()).empty()
                && !Misc::wxToString(ctrl.txtRule->GetValue()).empty()
                && !Misc::wxToString(ctrl.txtCommand->GetValue()).empty();
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

class BadRuleException: std::exception {};

void NewConflictRuleDialog::OnOK(wxCommandEvent&)
{
    try
    {
        auto rule = ConflictRule(
            std::string(Misc::wxToString(ctrl.txtName->GetValue())),
            std::string(Misc::wxToString(ctrl.txtRule->GetValue())),
            std::string(Misc::wxToString(ctrl.txtCommand->GetValue()))
        );

        if (rule.badRule)
            throw BadRuleException();

        ConflictRuleDBConnector db(Utils::UUIDToDBPath(Global::CurrentConfig().uuid), SQLite::OPEN_READWRITE);
        db.Insert(rule);
        GenericPopup("Successfully created a new conflict rule.").ShowModal();
    }
    catch(const BadRuleException&)
    {
        GenericPopup("Entered rule is not a valid pseudo-regex rule!").ShowModal();
        return;
    }
    catch(const DBConnectorStatic::DBException&)
    {
        GenericPopup("Failed to create a new conflict rule.").ShowModal();
        return;
    }
    catch(const std::exception& e)
    {
        GenericPopup("Failed to open conflict rule database.").ShowModal();
        return;
    }

    EndModal(wxID_OK);
}

void NewConflictRuleDialog::OnAnyChange(wxCommandEvent&)
{
    this->CheckIfOK();
}
