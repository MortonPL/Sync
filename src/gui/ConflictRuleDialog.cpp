#include "GUI/ConflictRuleDialog.h"

#include "GUI/Misc.h"
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

    selectedRuleIdx = -1;
    pRules = &rules;
    PopulateRuleList();
}

/*********************************** OTHER ***********************************/

void ConflictRuleDialog::PopulateRuleList()
{
    try
    {
        ConflictRuleDBConnector db(Utils::UUIDToDBPath(Global::CurrentConfig().uuid));
        pRules->clear();
        db.SelectAll(*pRules);
        auto arrs = wxArrayString();
        for(auto rule: *pRules)
            arrs.Add(Misc::stringToWx(rule.name));
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
    if (selectedRuleIdx < 0)
        return;

    auto rule = (*pRules)[selectedRuleIdx];
    ctrl.txtDetails->Clear();
    *ctrl.txtDetails << "Name:\t\t" << Misc::stringToWx(rule.name) << "\n";
    *ctrl.txtDetails << "Rule:\t\t" << Misc::stringToWx(rule.rule) << "\n";
    *ctrl.txtDetails << "Command:\t" << Misc::stringToWx(rule.command) << "\n";
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

void ConflictRuleDialog::OnListBoxChange(wxCommandEvent&)
{
    CheckIfRuleSelected();
}

void ConflictRuleDialog::OnNewRule(wxCommandEvent&)
{
    if (NewConflictRuleDialog().ShowModal() == wxID_OK)
        PopulateRuleList();
}

void ConflictRuleDialog::OnEditRule(wxCommandEvent&)
{
    if (EditConflictRuleDialog((*pRules)[selectedRuleIdx]).ShowModal() == wxID_OK)
    {
        PopulateRuleList();
        PopulateRuleDetails();
    }
}

void ConflictRuleDialog::OnMoveUpRule(wxCommandEvent&)
{
    if (selectedRuleIdx > 0)
    {
        try
        {
            ConflictRuleDBConnector db(Utils::UUIDToDBPath(Global::CurrentConfig().uuid), SQLite::OPEN_READWRITE);
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

void ConflictRuleDialog::OnMoveDownRule(wxCommandEvent&)
{
    if (selectedRuleIdx < (long)pRules->size() - 1)
    {
        try
        {
            ConflictRuleDBConnector db(Utils::UUIDToDBPath(Global::CurrentConfig().uuid), SQLite::OPEN_READWRITE);
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

void ConflictRuleDialog::OnDeleteRule(wxCommandEvent&)
{
    try
    {
        ConflictRuleDBConnector db(Utils::UUIDToDBPath(Global::CurrentConfig().uuid), SQLite::OPEN_READWRITE);
        if (db.Delete((*pRules)[selectedRuleIdx].id))
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

void ConflictRuleDialog::OnAuto(wxCommandEvent&)
{
    EndModal(CONFLICT_AUTO);
}

void ConflictRuleDialog::OnOK(wxCommandEvent&)
{
    EndModal(selectedRuleIdx);
}

void ConflictRuleDialog::OnCancel(wxCommandEvent&)
{
    EndModal(CONFLICT_CANCEL);
}
