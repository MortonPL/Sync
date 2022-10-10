# sources
set(SRC_UTILS_FILES
    ${SRC_HEADERS_FILES}
    Logger.h
    Logger.cpp
)

add_library(utils ${SRC_UTILS_FILES})
target_link_libraries(utils lib)
