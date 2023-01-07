# sources
set(SRC_UTILS_FILES 
    Utils.cpp
)

add_compile_options(-Wall -Wextra -pedantic -Werror)

add_library(utils ${SRC_UTILS_FILES})
target_include_directories(utils PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(utils xxHash::xxhash easyloggingpp fmt)
