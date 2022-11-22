# dependencies
find_package(wxWidgets REQUIRED COMPONENTS core base net xrc)
include(${wxWidgets_USE_FILE})

# sources
set(SRC_GUI_FILES
    # generics
    GenericPopup.cpp
    # custom dialog
    NewConfigurationDialog.cpp
    ChangeConfigurationDialog.cpp
    EditConfigurationDialog.cpp
    # frames
    MainFrame.cpp
    # main
    main.cpp
)

add_library(gui ${SRC_GUI_FILES})
target_include_directories(gui PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(gui ${wxWidgets_LIBRARIES} lib utils)
