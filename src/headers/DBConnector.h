#pragma once
#include "SQLite.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

#include "Domain/Configuration.h"

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
    static std::string Filename;
};
