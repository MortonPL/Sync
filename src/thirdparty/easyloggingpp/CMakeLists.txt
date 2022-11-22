# sources
set(SRC_THIRDPARTY_EASYLOGGINGPP_FILES 
    easylogging++.cc
)

add_definitions(-DELPP_NO_DEFAULT_LOG_FILE)
add_library(easyloggingpp ${SRC_THIRDPARTY_EASYLOGGINGPP_FILES})
