# sources
set(SRC_CLI_FILES
    Global.cpp
    # main
    main.cpp
)

add_compile_options(-Wall -Wextra -pedantic -Werror)

add_library(cli ${SRC_CLI_FILES})
target_include_directories(cli PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(cli lib utils)
