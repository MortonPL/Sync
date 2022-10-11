# sources
set(SRC_LIB_FILES
    SSHConnector.h
    SSHConnector.cpp
)

add_library(lib ${SRC_LIB_FILES})
target_link_libraries(lib utils headers)
