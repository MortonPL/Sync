# sources
set(SRC_CLI_FILES
    # main
    main.cpp
)

add_library(cli ${SRC_CLI_FILES})
target_link_libraries(cli lib utils headers)
