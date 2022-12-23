# dependencies
include(FetchContent)
FetchContent_Declare(
  SQLiteCpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG 3.2.0
)
FetchContent_MakeAvailable(SQLiteCpp)
FetchContent_Declare(
  xxHash
  GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
  GIT_TAG v0.8.1
  SOURCE_SUBDIR  cmake_unofficial
)
set(XXHASH_BUILD_XXHSUM OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(xxHash)
find_package(libssh REQUIRED)

# sources
set(SRC_LIB_FILES
    domain/Configuration.cpp
    domain/FileNode.cpp
    Global.cpp
    General.cpp
    SocketListener.cpp
    Creeper.cpp
    DBConnector.cpp
    SSHConnector.cpp
)

add_library(lib ${SRC_LIB_FILES})
target_include_directories(lib PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(lib ssh SQLiteCpp xxHash::xxhash uuid utils)
