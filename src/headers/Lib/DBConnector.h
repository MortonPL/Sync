#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>

#include "Domain/Configuration.h"
#include "Domain/FileNode.h"

#define NOID -1

class DBConnector
{
public:
    DBConnector(std::string path, int mode=SQLite::OPEN_READONLY);
    ~DBConnector();

    static std::string GetMainFileName();

    static bool EnsureCreatedMain();
    bool InsertConfig(Configuration config);
    bool UpdateConfig(Configuration config);
    bool DeleteConfig(int id);
    std::vector<Configuration> SelectAllConfigs();
    Configuration SelectConfigByUUID(std::string uuid);

    static bool EnsureCreatedHistory(std::string path);
    bool InsertFileNode(FileNode file);
    bool UpdateFileNode(FileNode file);
    bool DeleteFileNode(int id);
    std::vector<FileNode> SelectAllFileNodes();

private:
    SQLite::Database db;
};
