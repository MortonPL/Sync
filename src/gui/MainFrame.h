#pragma once
#include "../headers/wx.h"

class MainFrame : public wxFrame {
public:
    MainFrame(wxWindow* parent=(wxWindow*)NULL);

private:
    std::vector<wxPanel*> allPanels;
    wxPanel* pLeftPanel;
    wxPanel* pRightPanel;

    void OnExit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
