# sources
set(SRC_GUI_FILES
    DialogSSHAuth.h
    DialogSSHAuth.cpp
    MainFrame.h
    MainFrame.cpp
)

add_library(gui ${SRC_GUI_FILES})
target_link_libraries(gui lib utils headers)
