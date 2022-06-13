#pragma once
#include "wx.h"

class MainFrame : public wxFrame {
public:
    MainFrame(wxWindow* parent=(wxWindow*)NULL);

private:
    wxTextCtrl* pInput;
    wxTextCtrl* pOutput;

    // event handlers
    void OnHello(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
    void OnTextEnter(wxCommandEvent &event);
    void OnButtonConnect(wxCommandEvent &event);

    // other
    void HandleSSHTest(std::string hostname);
    bool HandleUserAuth(std::string &user, std::string &password);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
