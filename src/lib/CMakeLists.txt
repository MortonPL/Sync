# dependencies
include(FetchContent)
FetchContent_Declare(
  SQLiteCpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG 3.2.0
)
FetchContent_MakeAvailable(SQLiteCpp)
find_package(libssh REQUIRED)

# sources
set(SRC_LIB_FILES
    domain/Configuration.cpp
    Global.cpp
    DBConnector.cpp
    SSHConnector.cpp
)

add_library(lib ${SRC_LIB_FILES})
target_include_directories(lib PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(lib ssh SQLiteCpp utils)
