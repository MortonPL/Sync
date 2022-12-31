# directories
add_subdirectory(thirdparty)
add_subdirectory(utils)
add_subdirectory(lib)
add_subdirectory(gui)
add_subdirectory(cli)

# sources
set(SRC_FILES
    dummy.cpp
)

# add the library version
add_library(${LIB_NAME} ${SRC_FILES})
target_link_libraries(${LIB_NAME} lib utils uuid)

# add the GUI executable
add_executable(${GUI_NAME} ${SRC_FILES})
target_link_libraries(${GUI_NAME} gui ${LIB_NAME})
install(TARGETS ${GUI_NAME} DESTINATION bin)
if (${CMAKE_BUILD_TYPE} MATCHES RELEASE)
    install(FILES ../build/sync.xrs DESTINATION res)
elseif(${CMAKE_BUILD_TYPE} MATCHES DEBUG)
    install(FILES ../debug/sync.xrs DESTINATION res)
endif()

# add the CLI executable
add_executable(${CLI_NAME} ${SRC_FILES})
target_link_libraries(${CLI_NAME} cli ${LIB_NAME})
install(TARGETS ${CLI_NAME} DESTINATION bin)
