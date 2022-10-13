# sources
set(SRC_GUI_FILES
    # dialogs
    SSHAuthDialog.h
    SSHAuthDialog.cpp
    NewConfigurationDialog.h
    NewConfigurationDialog.cpp
    ChangeConfigurationDialog.h
    ChangeConfigurationDialog.cpp
    # frames
    MainFrame.h
    MainFrame.cpp
)

add_library(gui ${SRC_GUI_FILES})
target_link_libraries(gui lib utils headers)
