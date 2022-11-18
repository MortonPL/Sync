#pragma once
#include "wx.h"

class MainFrame : public wxFrame {
public:
    MainFrame(wxWindow* parent=(wxWindow*)NULL);

private:
    // control struct
    struct Controls {
        wxListCtrl* listMain;
    };

    // properties
    Controls ctrl;
    bool isFirstConfig;

    // custom methods
    void CreateReportList();

    // event handlers
    void Update();
    void OnNewConfig(wxCommandEvent &event);
    void OnChangeConfig(wxCommandEvent &event);
    void OnScan(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);

    // widget event table
    wxDECLARE_EVENT_TABLE();
};
