#pragma once
#include "SQLite.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>

#include "Domain/Configuration.h"

#define NOID -1

class DBConnector
{
public:
    DBConnector(int mode);
    ~DBConnector();

    static bool EnsureCreated();
    bool InsertConfig(Configuration config);
    bool UpdateConfig(Configuration config);
    bool DeleteConfig(int id);
    std::vector<Configuration> SelectAllConfigs();

private:
    SQLite::Database db;
    static std::string filename;
};
