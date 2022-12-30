#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>
#include <forward_list>

#include "Domain/Configuration.h"
#include "Domain/HistoryFileNode.h"

#define NOID -1
#define DB_FAIL 0
#define DB_EMPTY 1
#define DB_GOOD 2

/*A class that connects to a specific SQLite database file and makes CRUD queries.*/
class DBConnector
{
public:
    DBConnector(std::string path, int mode=SQLite::OPEN_READONLY);
    ~DBConnector();

    static std::string GetMainFileName();

    static int EnsureCreatedMain();
    bool InsertConfig(Configuration config);
    bool UpdateConfig(Configuration config);
    bool DeleteConfig(int id);
    std::vector<Configuration> SelectAllConfigs();

    static int EnsureCreatedHistory(std::string path);
    bool InsertFileNode(HistoryFileNode file);
    bool UpdateFileNode(HistoryFileNode file);
    bool DeleteFileNode(std::string& path);
    std::forward_list<HistoryFileNode> SelectAllFileNodes();

private:
    SQLite::Database db;
};
