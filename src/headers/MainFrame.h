#pragma once
#include "../headers/wx.h"

class MainFrame : public wxFrame {
public:
    MainFrame(wxWindow* parent=(wxWindow*)NULL);

private:
    // control struct
    struct Control {};

    // properties
    Control ctrl;
    wxPanel* pViewPanel;

    // event handlers
    void Update();
    void OnNewConfig(wxCommandEvent &event);
    void OnChangeConfig(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
