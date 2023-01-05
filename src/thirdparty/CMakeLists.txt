# directories
add_subdirectory(easyloggingpp)
add_subdirectory(sqlite)

# dependencies
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 9.1.0
)
FetchContent_MakeAvailable(fmt)

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
