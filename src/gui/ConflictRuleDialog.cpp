#include "GUI/ConflictRuleDialog.h"

#include "GUI/GenericPopup.h"
#include "GUI/NewConflictRuleDialog.h"
#include "Lib/DBConnector.h"
#include "Lib/Global.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(ConflictRuleDialog, wxDialog)
    EVT_LISTBOX(XRCID("listConflictRules"), ConflictRuleDialog::OnListBoxChange)
    EVT_LISTBOX_DCLICK(XRCID("listConflictRules"), ConflictRuleDialog::OnOK)
    EVT_BUTTON(XRCID("btnNew"), ConflictRuleDialog::OnNewRule)
    EVT_BUTTON(XRCID("btnEdit"), ConflictRuleDialog::OnEditRule)
    EVT_BUTTON(XRCID("btnMoveUp"), ConflictRuleDialog::OnMoveUpRule)
    EVT_BUTTON(XRCID("btnMoveDown"), ConflictRuleDialog::OnMoveDownRule)
    EVT_BUTTON(XRCID("btnDelete"), ConflictRuleDialog::OnDeleteRule)
    EVT_BUTTON(XRCID("btnAuto"), ConflictRuleDialog::OnAuto)
    EVT_BUTTON(wxID_OK, ConflictRuleDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ConflictRuleDialog::OnCancel)
wxEND_EVENT_TABLE()

// ctor
ConflictRuleDialog::ConflictRuleDialog(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "ConflictRuleDialog");

    ctrl = Controls
    {
        (wxListBox*)(FindWindow("listConflictRules")),
        (wxButton*)(FindWindow("btnNew")),
        (wxButton*)(FindWindow("btnEdit")),
        (wxButton*)(FindWindow("btnMoveUp")),
        (wxButton*)(FindWindow("btnMoveDown")),
        (wxButton*)(FindWindow("btnDelete")),
        (wxTextCtrl*)(FindWindow("txtDetails")),
        (wxButton*)(FindWindow("btnAuto")),
        (wxButton*)(FindWindow("wxID_OK")),
        (wxButton*)(FindWindow("wxID_CANCEL")),
    };

    this->selectedRuleIdx = -1;
    PopulateRuleList();
}

/*********************************** OTHER ***********************************/

void ConflictRuleDialog::PopulateRuleList()
{
    DBConnector db(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid));
    try
    {
        this->rules.clear();
        db.SelectAllConflictRules(this->rules);
        auto arrs = wxArrayString();
        for(auto rule: this->rules)
            arrs.Add(wxString::FromUTF8(rule.name));
        ctrl.listConflictRules->Clear();
        if (!arrs.IsEmpty())
            ctrl.listConflictRules->InsertItems(arrs, 0);
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        GenericPopup("Failed to open/refresh conflict rule database.").ShowModal();
        return;
    }
}

void ConflictRuleDialog::PopulateRuleDetails()
{
    if (this->selectedRuleIdx < 0)
        return;

    auto rule = this->rules[this->selectedRuleIdx];
    ctrl.txtDetails->Clear();
    *ctrl.txtDetails << "Name:\t\t" << wxString::FromUTF8(rule.name) << "\n";
    *ctrl.txtDetails << "Rule:\t\t" << wxString::FromUTF8(rule.rule) << "\n";
    *ctrl.txtDetails << "Command:\t" << wxString::FromUTF8(rule.command) << "\n";
}

void ConflictRuleDialog::CheckIfRuleSelected()
{
    selectedRuleIdx = ctrl.listConflictRules->GetSelection();
    if (selectedRuleIdx != wxNOT_FOUND)
    {
        PopulateRuleDetails();
        ctrl.btnEdit->Enable();
        ctrl.btnMoveUp->Enable();
        ctrl.btnMoveDown->Enable();
        ctrl.btnDelete->Enable();
        ctrl.btnOK->Enable();
    }
    else
    {
        ctrl.txtDetails->Clear();
        ctrl.btnEdit->Disable();
        ctrl.btnMoveUp->Disable();
        ctrl.btnMoveDown->Disable();
        ctrl.btnDelete->Disable();
        ctrl.btnOK->Disable();
    }
}

/******************************* EVENT HANDLERS ******************************/

void ConflictRuleDialog::OnListBoxChange(wxCommandEvent &event)
{
    CheckIfRuleSelected();
}

void ConflictRuleDialog::OnNewRule(wxCommandEvent &event)
{
    if (NewConflictRuleDialog().ShowModal() == wxID_OK)
        PopulateRuleList();
}

void ConflictRuleDialog::OnEditRule(wxCommandEvent &event)
{
    // TODO
    if (GenericPopup(rules[selectedRuleIdx].name).ShowModal() == wxID_OK)
    //if (EditConflictRuleDialog(rules[selectedRuleIdx]).ShowModal() == wxID_OK)
    {
        PopulateRuleList();
        PopulateRuleDetails();
    }
}

void ConflictRuleDialog::OnMoveUpRule(wxCommandEvent &event)
{
    if (selectedRuleIdx > 0)
    {
        try
        {
            DBConnector db(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid), SQLite::OPEN_READWRITE);
            if (db.SwapConflictRule(rules[selectedRuleIdx - 1], rules[selectedRuleIdx]))
            {
                ctrl.listConflictRules->SetString(selectedRuleIdx - 1, rules[selectedRuleIdx].name);
                ctrl.listConflictRules->SetString(selectedRuleIdx, rules[selectedRuleIdx - 1].name);
                std::swap(rules[selectedRuleIdx - 1], rules[selectedRuleIdx]);
                ctrl.listConflictRules->Select(selectedRuleIdx - 1);
                selectedRuleIdx--;
            }
            else
            {
                GenericPopup("Failed to move conflict rule.").ShowModal();
                return;
            }
        }
        catch(const std::exception& e)
        {
            LOG(ERROR) << e.what();
            GenericPopup("Failed to open conflict rule database.").ShowModal();
            return;
        }
    }
}

void ConflictRuleDialog::OnMoveDownRule(wxCommandEvent &event)
{
    if (selectedRuleIdx < rules.size() - 1)
    {
        try
        {
            DBConnector db(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid), SQLite::OPEN_READWRITE);
            if (db.SwapConflictRule(rules[selectedRuleIdx + 1], rules[selectedRuleIdx]))
            {
                ctrl.listConflictRules->SetString(selectedRuleIdx + 1, rules[selectedRuleIdx].name);
                ctrl.listConflictRules->SetString(selectedRuleIdx, rules[selectedRuleIdx + 1].name);
                std::swap(rules[selectedRuleIdx + 1], rules[selectedRuleIdx]);
                ctrl.listConflictRules->Select(selectedRuleIdx + 1);
                selectedRuleIdx++;
            }
            else
            {
                GenericPopup("Failed to move conflict rule.").ShowModal();
                return;
            }
        }
        catch(const std::exception& e)
        {
            LOG(ERROR) << e.what();
            GenericPopup("Failed to open conflict rule database.").ShowModal();
            return;
        }
    }
}

void ConflictRuleDialog::OnDeleteRule(wxCommandEvent &event)
{
    try
    {
        DBConnector db(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid), SQLite::OPEN_READWRITE);
        if (db.DeleteConflictRule(rules[selectedRuleIdx].id))
        {
            GenericPopup("Conflict rules removed successfully.").ShowModal();
        }
        else
        {
            GenericPopup("Failed to remove conflict rule.").ShowModal();
            return;
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << e.what();
        GenericPopup("Failed to open conflict rule database.").ShowModal();
        return;
    }

    PopulateRuleList();
    CheckIfRuleSelected();
}

void ConflictRuleDialog::OnAuto(wxCommandEvent &event)
{
    //Global::SetCurrentConfig(configs[selectedConfigIdx]);
    EndModal(wxID_OK);
}

void ConflictRuleDialog::OnOK(wxCommandEvent &event)
{
    //Global::SetCurrentConfig(configs[selectedConfigIdx]);
    EndModal(wxID_OK);
}

void ConflictRuleDialog::OnCancel(wxCommandEvent &event)
{
    EndModal(wxID_CANCEL);
}
