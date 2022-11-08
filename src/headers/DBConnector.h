#pragma once
#include "SQLite.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

#include "Configuration.h"

#define DATABASE_NAME "Sync.db3"

class DBConnector
{
public:
    DBConnector();
    ~DBConnector();

    static bool EnsureCreated();
    bool Open(int mode=SQLite::OPEN_READONLY);
    bool Close();

    bool InsertConfig(Configuration config);
    Configuration SelectConfig();
private:
    SQLite::Database* db;
};
