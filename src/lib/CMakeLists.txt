# dependencies
find_package(libssh REQUIRED)

# sources
set(SRC_LIB_FILES
    domain/Configuration.cpp
    domain/FileNode.cpp
    domain/HistoryFileNode.cpp
    domain/PairedNode.cpp
    domain/ConflictRule.cpp
    Compression.cpp
    Announcer.cpp
    Global.cpp
    FileSystem.cpp
    General.cpp
    SocketListener.cpp
    Blocker.cpp
    Mapper.cpp
    Creeper.cpp
    PairingManager.cpp
    SyncManager.cpp
    ConflictManager.cpp
    DBConnector.cpp
    SSHConnector.cpp
    SFTPConnector.cpp
)

add_compile_options(-Wall -Wextra -pedantic -Werror)

add_library(lib ${SRC_LIB_FILES})
target_include_directories(lib PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(lib ssh SQLiteCpp zstd xxHash::xxhash uuid utils)
