#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>

#include "Domain/Configuration.h"
#include "Domain/FileNode.h"

#define NOID -1
#define DB_FAIL 0
#define DB_EMPTY 1
#define DB_GOOD 2

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
    Configuration SelectConfigByUUID(std::string uuid);

    static int EnsureCreatedHistory(std::string path);
    bool InsertFileNode(FileNode file);
    bool UpdateFileNode(FileNode file);
    bool DeleteFileNode(std::string& path);
    std::vector<FileNode> SelectAllFileNodes();

private:
    SQLite::Database db;
};
