#include "MainFrame.h"
#include "SSHConnector.h"
#include "DialogSSHAuth.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(XRCID("menu_file_hello"), MainFrame::OnHello)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_TEXT_ENTER(wxID_ANY, MainFrame::OnTextEnter)
    EVT_BUTTON(XRCID("m_button6"), MainFrame::OnButtonConnect)
wxEND_EVENT_TABLE()

// ctor
MainFrame::MainFrame(wxWindow* pParent)
{
    wxXmlResource::Get()->LoadFrame(this, pParent, "main_frame");

    pInput = XRCCTRL(*this, "m_textCtrl4", wxTextCtrl);
    pOutput = XRCCTRL(*this, "m_textCtrl5", wxTextCtrl);

    SetMenuBar(wxXmlResource::Get()->LoadMenuBar(this, "main_menu"));

    CreateStatusBar();
    SetStatusText("This is the status bar!");

    // resize for menu and status bar
    GetSizer()->SetSizeHints(this);
}

/********************************* RESOURCES *********************************/

/******************************* EVENT HANDLERS ******************************/

void MainFrame::OnHello(wxCommandEvent &event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MainFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox("This is a message box.", "About...",
                wxOK | wxICON_INFORMATION);
}

void MainFrame::OnExit(wxCommandEvent &event)
{
    Close(true);
}

bool MainFrame::HandleUserAuth(std::string &user, std::string &password)
{
    DialogSSHAuth dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
        return false;
    }
    user = dialog.userValue;
    password = dialog.passValue;
    return true;
}

void MainFrame::HandleSSHTest(std::string hostname)
{
    bool r = true;
    std::string result = "";

    auto ssh = SSHConnector();
    r = ssh.BeginSession(hostname);
    r = ssh.AuthenticateServer();
    
    std::string user;
    std::string password;
    if(!HandleUserAuth(user, password))
    {
        SetStatusText("Canceled.");
        ssh.EndSession();
        return;
    }

    r = ssh.AuthenticateUserPass(user, password);
    r = ssh.ExecuteLS(result);
    ssh.EndSession();

    this->pOutput->ChangeValue(result);
}

void MainFrame::OnTextEnter(wxCommandEvent &event)
{
    HandleSSHTest(this->pInput->GetValue().ToStdString());
    
}

void MainFrame::OnButtonConnect(wxCommandEvent &event)
{
    HandleSSHTest(this->pInput->GetValue().ToStdString());
}
