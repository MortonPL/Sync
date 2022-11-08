# dependencies
include(FetchContent)
FetchContent_Declare(
  SQLiteCpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG 3.2.0
)
FetchContent_MakeAvailable(SQLiteCpp)

# sources
set(SRC_LIB_FILES
    Configuration.cpp
    DBConnector.cpp
    SSHConnector.cpp
)

add_library(lib ${SRC_LIB_FILES})
target_link_libraries(lib SQLiteCpp utils headers)
