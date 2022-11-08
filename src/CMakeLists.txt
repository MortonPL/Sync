# dependencies
find_package(wxWidgets REQUIRED COMPONENTS net core base xrc)
find_package(libssh REQUIRED)
include(${wxWidgets_USE_FILE})

# directories
add_subdirectory(headers)
add_subdirectory(thirdparty)
add_subdirectory(utils)
add_subdirectory(lib)
add_subdirectory(gui)
add_subdirectory(cli)

# sources
set(SRC_FILES
    # main
    dummy.cpp
)

# add the GUI executable
add_executable(${GUI_NAME} ${SRC_FILES})
target_link_libraries(${GUI_NAME} headers gui lib utils)
target_link_libraries(${GUI_NAME} ${wxWidgets_LIBRARIES} ssh)

# add the CLI executable
add_executable(${CLI_NAME} ${SRC_FILES})
target_link_libraries(${CLI_NAME} headers cli lib utils)
target_link_libraries(${CLI_NAME} ssh)

# add the library version
add_library(${LIB_NAME} ${SRC_FILES})
target_link_libraries(${LIB_NAME} headers gui cli lib utils)
target_link_libraries(${LIB_NAME} ${wxWidgets_LIBRARIES} ssh)
