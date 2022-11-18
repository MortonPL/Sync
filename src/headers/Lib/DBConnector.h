#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>

#include "Domain/Configuration.h"

#define NOID -1

class DBConnector
{
public:
    DBConnector(std::string filename, int mode);
    ~DBConnector();

    static std::string& GetMainFileName();

    static bool EnsureCreated();
    bool InsertConfig(Configuration config);
    bool UpdateConfig(Configuration config);
    bool DeleteConfig(int id);
    std::vector<Configuration> SelectAllConfigs();

private:
    SQLite::Database db;
    static std::string mainFile;
};
