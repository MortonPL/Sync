# dependencies
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 9.1.0
)
FetchContent_MakeAvailable(fmt)

# sources
set(SRC_UTILS_FILES 
    Utils.cpp
)

add_library(utils ${SRC_UTILS_FILES})
target_include_directories(utils PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(utils easyloggingpp fmt)
