# directories
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
target_link_libraries(${GUI_NAME} gui lib utils)

# add the CLI executable
add_executable(${CLI_NAME} ${SRC_FILES})
target_link_libraries(${CLI_NAME} cli lib utils)

# add the library version
add_library(${LIB_NAME} ${SRC_FILES})
target_link_libraries(${LIB_NAME} gui cli lib utils)
