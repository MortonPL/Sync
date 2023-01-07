#include "GUI/EditConfigurationDialog.h"

#include "GUI/GenericPopup.h"
#include "Lib/DBConnector.h"
#include "Utils.h"

wxBEGIN_EVENT_TABLE(EditConfigurationDialog, wxDialog)
    EVT_BUTTON(wxID_OK, EditConfigurationDialog::OnOK)
    EVT_TEXT(XRCID("txtConfigName"), EditConfigurationDialog::OnAnyChange)
    EVT_DIRPICKER_CHANGED(XRCID("dirRootA"), EditConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtAddressB"), EditConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtUserB"), EditConfigurationDialog::OnAnyChange)
    EVT_TEXT(XRCID("txtRootB"), EditConfigurationDialog::OnAnyChange)
wxEND_EVENT_TABLE()

// ctor
EditConfigurationDialog::EditConfigurationDialog(Configuration& oldConfig, wxWindow* pParent)
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

    this->oldConfig = oldConfig;

    //fill in controls
    ctrl.txtConfigName->AppendText(wxString::FromUTF8(oldConfig.name));
    ctrl.dirRootA->SetPath(wxString::FromUTF8(oldConfig.pathA));
    ctrl.txtRootB->AppendText(wxString::FromUTF8(oldConfig.pathB));
    ctrl.txtAddressB->AppendText(wxString::FromUTF8(oldConfig.pathBaddress));
    ctrl.txtUserB->AppendText(wxString::FromUTF8(oldConfig.pathBuser));

    ctrl.btnOK->SetLabel("Edit");
    ctrl.btnOK->Disable();
}

void EditConfigurationDialog::CheckIfOK()
{
    bool isOK = !ctrl.txtConfigName->GetValue().utf8_string().empty()
                && !ctrl.dirRootA->GetPath().utf8_string().empty()
                && !ctrl.txtRootB->GetValue().utf8_string().empty()
                && !ctrl.txtAddressB->GetValue().utf8_string().empty()
                && !ctrl.txtUserB->GetValue().utf8_string().empty();
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

void EditConfigurationDialog::OnOK(wxCommandEvent &event)
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
    config = Configuration(
        oldConfig.id,
        ctrl.txtConfigName->GetValue().utf8_string(),
        oldConfig.uuid,
        oldConfig.timestamp,
        Utils::CorrectDirPath(pathA),
        Utils::CorrectDirPath(pathB),
        ctrl.txtAddressB->GetValue().utf8_string(),
        ctrl.txtUserB->GetValue().utf8_string(),
        0
    );

    if (config == oldConfig)
    {
        EndModal(wxID_CANCEL);
        return;
    }

    try
    {
        DBConnector db(DBConnector::GetMainFileName(), SQLite::OPEN_READWRITE);
        db.UpdateConfig(config);
    }
    catch(const std::exception& e)
    {
        GenericPopup("Failed to open configuration database.").ShowModal();
        return;
    }
    GenericPopup("Successfully edited the configuration.").ShowModal();

    EndModal(wxID_OK);
}

void EditConfigurationDialog::OnAnyChange(wxCommandEvent &event)
{
    event.GetId(); //unused
    this->CheckIfOK();
}

void EditConfigurationDialog::OnAnyChange(wxFileDirPickerEvent &event)
{
    event.GetId(); //unused
    this->CheckIfOK();
}
