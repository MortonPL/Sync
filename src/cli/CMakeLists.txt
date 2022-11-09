# sources
set(SRC_CLI_FILES
    # main
    main.cpp
)

add_library(cli ${SRC_CLI_FILES})
target_include_directories(cli PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(cli lib utils)
