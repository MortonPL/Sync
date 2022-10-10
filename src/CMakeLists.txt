find_package(wxWidgets REQUIRED COMPONENTS net core base xrc)
find_package(libssh REQUIRED)
include(${wxWidgets_USE_FILE})

# sources
set(SRC_FILES
    # headers
    wx.h
    # codes
    SSHConnector.h
    SSHConnector.cpp
    # GUIs
    DialogSSHAuth.h
    DialogSSHAuth.cpp
    MainFrame.h
    MainFrame.cpp
    # main
    main.cpp
)

# add the executable
add_executable(${EXE_NAME} ${SRC_FILES})
add_library(${LIB_NAME} ${SRC_FILES})
target_link_libraries(${EXE_NAME} ${wxWidgets_LIBRARIES} ssh)
target_link_libraries(${LIB_NAME} ${wxWidgets_LIBRARIES} ssh)
