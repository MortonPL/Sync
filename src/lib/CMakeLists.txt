# sources
set(SRC_LIB_FILES
    SSHConnector.cpp
)

add_library(lib ${SRC_LIB_FILES})
target_link_libraries(lib utils headers)
