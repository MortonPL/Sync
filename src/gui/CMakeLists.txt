# dependencies
find_package(wxWidgets REQUIRED COMPONENTS core base net xrc)
include(${wxWidgets_USE_FILE})

# sources
set(SRC_GUI_FILES
    # generics
    GenericPopup.cpp
    # utilities
    SSHConnectorWrap.cpp
    Misc.cpp
    # custom dialog
    NewConfigurationDialog.cpp
    EditConfigurationDialog.cpp
    ChangeConfigurationDialog.cpp
    ConflictRuleDialog.cpp
    NewConflictRuleDialog.cpp
    EditConflictRuleDialog.cpp
    # frames
    MainFrame.cpp
    # main
    main.cpp
)

add_compile_options(-Wall -Wextra -pedantic -Werror)

add_library(gui ${SRC_GUI_FILES})
target_include_directories(gui PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(gui ${wxWidgets_LIBRARIES} lib utils)
