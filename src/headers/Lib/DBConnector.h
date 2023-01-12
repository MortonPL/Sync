#ifndef LIB_DBCONNECTOR_H
#define LIB_DBCONNECTOR_H
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>
#include <forward_list>

#include "Domain/Configuration.h"
#include "Domain/HistoryFileNode.h"
#include "Domain/ConflictRule.h"

#define NOID -1
/*A class that connects to a specific SQLite database file and makes CRUD queries.*/
class DBConnector
{
public:
    DBConnector(std::string path, int mode=SQLite::OPEN_READONLY);
    ~DBConnector();

    static std::string GetMainFileName();

    static bool EnsureCreatedMain();
    bool InsertConfig(const Configuration config);
    bool UpdateConfig(const Configuration config);
    bool DeleteConfig(int id);
    void SelectAllConfigs(std::vector<Configuration>& configs);

    static bool EnsureCreatedHistory(const std::string path);
    bool InsertFileNode(const HistoryFileNode& file);
    bool UpdateFileNode(const HistoryFileNode& file);
    bool DeleteFileNode(const std::string path);
    void SelectAllFileNodes(std::forward_list<HistoryFileNode>& nodes);

    bool InsertConflictRule(const ConflictRule& rule);
    bool UpdateConflictRule(const ConflictRule& rule);
    bool DeleteConflictRule(int id);
    bool SwapConflictRule(const ConflictRule& rule1, const ConflictRule& rule2);
    void SelectAllConflictRules(std::vector<ConflictRule>& nodes);

private:
    SQLite::Database db;
};

#endif
