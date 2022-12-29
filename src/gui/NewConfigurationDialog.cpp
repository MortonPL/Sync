#include "GUI/NewConfigurationDialog.h"

#include <uuid/uuid.h>
#include "GUI/GenericPopup.h"
#include "Lib/DBConnector.h"
#include "Domain/Configuration.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(NewConfigurationDialog, wxDialog)
    EVT_BUTTON(wxID_OK, NewConfigurationDialog::OnOK)
    EVT_TEXT(XRCID("txtConfigName"), NewConfigurationDialog::OnAnyChange)
    EVT_DIRPICKER_CHANGED(XRCID("dirRootA"), NewConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtAddressB"), NewConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtUserB"), NewConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtRootB"), NewConfigurationDialog::OnAnyChange)
wxEND_EVENT_TABLE()

// ctor
NewConfigurationDialog::NewConfigurationDialog(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadDialog(this, pParent, "NewConfigurationDialog");

    ctrl = Controls
    {
        (wxTextCtrl*)(FindWindow("txtConfigName")),
        (wxDirPickerCtrl*)(FindWindow("dirRootA")),
        (wxTextCtrl*)(FindWindow("txtAddressB")),
        (wxTextCtrl*)(FindWindow("txtUserB")),
        (wxTextCtrl*)(FindWindow("txtRootB")),
        (wxButton*)(FindWindow("wxID_OK")),
        (wxButton*)(FindWindow("wxID_CANCEL")),
    };
}

void NewConfigurationDialog::CheckIfOK()
{
    bool isOK = !ctrl.txtConfigName->GetValue().IsEmpty()
                && !ctrl.dirRootA->GetPath().ToStdString().empty()
                && !ctrl.txtRootB->GetValue().IsEmpty()
                && !ctrl.txtAddressB->GetValue().IsEmpty()
                && !ctrl.txtUserB->GetValue().IsEmpty();
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

void NewConfigurationDialog::OnOK(wxCommandEvent &event)
{
    std::string pathA = ctrl.dirRootA->GetPath().ToStdString();
    std::string pathB = ctrl.txtRootB->GetValue().ToStdString();
    if (pathA.empty() || pathB.empty())
    {
        GenericPopup("Root paths cannot be empty!").ShowModal();
        return;
    }

    Configuration config;
    uuid_t uuid;
    uuid_generate(uuid);

    config = Configuration(
        NOID,
        ctrl.txtConfigName->GetValue().ToStdString(),
        uuid,
        Utils::CorrectDirPath(pathA),
        Utils::CorrectDirPath(pathB),
        ctrl.txtAddressB->GetValue().ToStdString(),
        ctrl.txtUserB->GetValue().ToStdString()
    );

    try
    {
        DBConnector db(DBConnector::GetMainFileName(), SQLite::OPEN_READWRITE);
        db.InsertConfig(config);
    }
    catch(const std::exception& e)
    {
        GenericPopup("Failed to open configuration database.").ShowModal();
        return;
    }
    GenericPopup("Successfully created a new configuration.").ShowModal();

    EndModal(wxID_OK);
}

void NewConfigurationDialog::OnAnyChange(wxCommandEvent &event)
{
    this->CheckIfOK();
}

void NewConfigurationDialog::OnAnyChange(wxFileDirPickerEvent &event)
{
    this->CheckIfOK();
}
