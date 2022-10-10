find_package(wxWidgets REQUIRED COMPONENTS net core base xrc)
find_package(libssh REQUIRED)
include(${wxWidgets_USE_FILE})

# directories
add_subdirectory(headers)
add_subdirectory(utils)
add_subdirectory(lib)
add_subdirectory(gui)

# sources
set(SRC_FILES
    # main
    main.cpp
)

# add the executable
add_executable(${EXE_NAME} ${SRC_FILES})
target_link_libraries(${EXE_NAME} headers gui lib utils)
target_link_libraries(${EXE_NAME} ${wxWidgets_LIBRARIES} ssh)

# add the library version
add_library(${LIB_NAME} ${SRC_FILES})
target_link_libraries(${LIB_NAME} headers gui lib utils)
target_link_libraries(${LIB_NAME} ${wxWidgets_LIBRARIES} ssh)
