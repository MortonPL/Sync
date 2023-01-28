#include "GUI/EditConfigurationDialog.h"

#include "GUI/Misc.h"
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
    ctrl.txtConfigName->AppendText(Misc::stringToWx(oldConfig.name));
    ctrl.dirRootA->SetPath(Misc::stringToWx(oldConfig.pathA));
    ctrl.txtRootB->AppendText(Misc::stringToWx(oldConfig.pathB));
    ctrl.txtAddressB->AppendText(Misc::stringToWx(oldConfig.pathBaddress));
    ctrl.txtUserB->AppendText(Misc::stringToWx(oldConfig.pathBuser));

    ctrl.btnOK->SetLabel("Edit");
    ctrl.btnOK->Disable();
}

void EditConfigurationDialog::CheckIfOK()
{
    const bool isOK = !Misc::wxToString(ctrl.txtConfigName->GetValue()).empty()
                   && !Misc::wxToString(ctrl.dirRootA->GetPath()).empty()
                   && !Misc::wxToString(ctrl.txtRootB->GetValue()).empty()
                   && !Misc::wxToString(ctrl.txtAddressB->GetValue()).empty()
                   && !Misc::wxToString(ctrl.txtUserB->GetValue()).empty();
    ctrl.btnOK->Enable(isOK);
}

/******************************* EVENT HANDLERS ******************************/

class EmptyPathException: std::exception{};

void EditConfigurationDialog::OnOK(wxCommandEvent&)
{
    std::string pathA = Misc::wxToString(ctrl.dirRootA->GetPath());
    std::string pathB = Misc::wxToString(ctrl.txtRootB->GetValue());
    if (pathA.empty() || pathB.empty())
        throw EmptyPathException();

    Configuration config;
    config = Configuration(
        oldConfig.id,
        Misc::wxToString(ctrl.txtConfigName->GetValue()),
        oldConfig.uuid,
        oldConfig.timestamp,
        Utils::CorrectDirPath(pathA),
        Utils::CorrectDirPath(pathB),
        Misc::wxToString(ctrl.txtAddressB->GetValue()),
        Misc::wxToString(ctrl.txtUserB->GetValue()),
        0
    );

    if (config == oldConfig)
    {
        EndModal(wxID_CANCEL);
        return;
    }

    try
    {
        ConfigurationDBConnector db(DBConnectorStatic::GetMainFileName(), SQLite::OPEN_READWRITE);
        db.Update(config);
    }
    catch(const DBConnectorStatic::DBException&)
    {
        GenericPopup("Failed to update the configuration.").ShowModal();
        return;
    }
    catch(const EmptyPathException&)
    {
        GenericPopup("Root paths cannot be empty!").ShowModal();
        return;
    }
    catch(const std::exception& e)
    {
        GenericPopup("Failed to open configuration database.").ShowModal();
        return;
    }
    GenericPopup("Successfully edited the configuration.").ShowModal();

    EndModal(wxID_OK);
}

void EditConfigurationDialog::OnAnyChange(wxCommandEvent&)
{
    this->CheckIfOK();
}

void EditConfigurationDialog::OnAnyChange(wxFileDirPickerEvent&)
{
    this->CheckIfOK();
}
