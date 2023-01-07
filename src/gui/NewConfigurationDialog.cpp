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
    bool isOK = !ctrl.txtConfigName->GetValue().utf8_string().empty()
                && !ctrl.dirRootA->GetPath().utf8_string().empty()
                && !ctrl.txtRootB->GetValue().utf8_string().empty()
                && !ctrl.txtAddressB->GetValue().utf8_string().empty()
                && !ctrl.txtUserB->GetValue().utf8_string().empty();
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

void NewConfigurationDialog::OnOK(wxCommandEvent &event)
{
    event.GetId(); //unused
    std::string pathA = ctrl.dirRootA->GetPath().utf8_string();
    std::string pathB = ctrl.txtRootB->GetValue().utf8_string();
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
        ctrl.txtConfigName->GetValue().utf8_string(),
        uuid,
        Utils::CorrectDirPath(pathA),
        Utils::CorrectDirPath(pathB),
        ctrl.txtAddressB->GetValue().utf8_string(),
        ctrl.txtUserB->GetValue().utf8_string()
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
    event.GetId(); //unused
    this->CheckIfOK();
}

void NewConfigurationDialog::OnAnyChange(wxFileDirPickerEvent &event)
{
    event.GetId(); //unused
    this->CheckIfOK();
}
