# sources
set(SRC_UTILS_FILES
    Logger.h
    Logger.cpp
)

add_library(utils ${SRC_UTILS_FILES})
target_link_libraries(utils headers)
