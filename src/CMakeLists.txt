find_package(wxWidgets REQUIRED COMPONENTS net core base xrc)
find_package(libssh REQUIRED)
include(${wxWidgets_USE_FILE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# sources
set(SRC_FILES
    # headers
    wx.h
    enums.h
    # codes
    SSHConnector.h
    SSHConnector.cpp
    # GUIs
    DialogSSHAuth.h
    DialogSSHAuth.cpp
    MainFrame.h
    MainFrame.cpp
    # main
    proto.cpp
)

# add the executable
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} ssh)
