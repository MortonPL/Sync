#include "GUI/ConflictRuleDialog.h"

#include "GUI/GenericPopup.h"
#include "GUI/NewConflictRuleDialog.h"
#include "GUI/EditConflictRuleDialog.h"
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
ConflictRuleDialog::ConflictRuleDialog(std::vector<ConflictRule>& rules, wxWindow* pParent)
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
    pRules = &rules;
    PopulateRuleList();
}

/*********************************** OTHER ***********************************/

void ConflictRuleDialog::PopulateRuleList()
{
    DBConnector db(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid));
    try
    {
        this->pRules->clear();
        db.SelectAllConflictRules(*this->pRules);
        auto arrs = wxArrayString();
        for(auto rule: *this->pRules)
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

    auto rule = (*this->pRules)[this->selectedRuleIdx];
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
    if (EditConflictRuleDialog((*pRules)[selectedRuleIdx]).ShowModal() == wxID_OK)
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
            if (db.SwapConflictRule((*pRules)[selectedRuleIdx - 1], (*pRules)[selectedRuleIdx]))
            {
                ctrl.listConflictRules->SetString(selectedRuleIdx - 1, (*pRules)[selectedRuleIdx].name);
                ctrl.listConflictRules->SetString(selectedRuleIdx, (*pRules)[selectedRuleIdx - 1].name);
                std::swap((*pRules)[selectedRuleIdx - 1], (*pRules)[selectedRuleIdx]);
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
    if (selectedRuleIdx < pRules->size() - 1)
    {
        try
        {
            DBConnector db(Utils::UUIDToDBPath(Global::GetCurrentConfig().uuid), SQLite::OPEN_READWRITE);
            if (db.SwapConflictRule((*pRules)[selectedRuleIdx + 1], (*pRules)[selectedRuleIdx]))
            {
                ctrl.listConflictRules->SetString(selectedRuleIdx + 1, (*pRules)[selectedRuleIdx].name);
                ctrl.listConflictRules->SetString(selectedRuleIdx, (*pRules)[selectedRuleIdx + 1].name);
                std::swap((*pRules)[selectedRuleIdx + 1], (*pRules)[selectedRuleIdx]);
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
        if (db.DeleteConflictRule((*pRules)[selectedRuleIdx].id))
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
    EndModal(CONFLICT_AUTO);
}

void ConflictRuleDialog::OnOK(wxCommandEvent &event)
{
    EndModal(selectedRuleIdx);
}

void ConflictRuleDialog::OnCancel(wxCommandEvent &event)
{
    EndModal(CONFLICT_CANCEL);
}
