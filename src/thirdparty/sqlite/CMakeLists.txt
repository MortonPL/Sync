# sources
set(SRC_THIRDPARTY_SQLITE_FILES 
    sqlite3.c
    shell.c
)

add_library(sqlite ${SRC_THIRDPARTY_SQLITE_FILES})
