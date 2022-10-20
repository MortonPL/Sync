# sources
set(SRC_GUI_FILES
    # generics
    GenericPopup.cpp
    # custom dialog
    NewConfigurationDialog.cpp
    ChangeConfigurationDialog.cpp
    # frames
    MainFrame.cpp
)

add_library(gui ${SRC_GUI_FILES})
target_link_libraries(gui lib utils headers)
