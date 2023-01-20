#ifndef LIB_DBCONNECTOR_H
#define LIB_DBCONNECTOR_H
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>
#include <forward_list>

#include "Domain/Configuration.h"
#include "Domain/HistoryFileNode.h"
#include "Domain/ConflictRule.h"
#include "Utils.h"

class DBConnectorStatic
{
public:
    static std::string GetMainFileName();
    static bool EnsureCreatedMain();
    static bool EnsureCreatedHistory(const std::string path);
    static const int NoID = -1;
};
template <typename TObject, typename TId, template<class, class> class TContainter>
class DBConnector
{
public:
    DBConnector(std::string path, int mode=SQLite::OPEN_READONLY): db(Utils::GetDatabasePath() + path, mode) {};
    virtual ~DBConnector() {};

    virtual bool Insert(const TObject& object) = 0;
    virtual bool Update(const TObject& object) = 0;
    virtual bool Delete(TId id) = 0;
    virtual void SelectAll(TContainter<TObject, std::allocator<TObject>>& objects) = 0;
protected:
    SQLite::Database db;
};

class ConfigurationDBConnector: DBConnector<Configuration, int, std::vector>
{
public:
    ConfigurationDBConnector(std::string path, int mode=SQLite::OPEN_READONLY): DBConnector(path, mode) {};

    bool Insert(const Configuration& config);
    bool Update(const Configuration& config);
    bool Delete(int id);
    void SelectAll(std::vector<Configuration>& configs);
};

class HistoryFileNodeDBConnector: DBConnector<HistoryFileNode, std::string, std::forward_list>
{
public:
    HistoryFileNodeDBConnector(std::string path, int mode=SQLite::OPEN_READONLY): DBConnector(path, mode) {};

    bool Insert(const HistoryFileNode& fileNode);
    bool Update(const HistoryFileNode& fileNode);
    bool Delete(std::string path);
    void SelectAll(std::forward_list<HistoryFileNode>& fileNodes);
};

class ConflictRuleDBConnector: DBConnector<ConflictRule, int, std::vector>
{
public:
    ConflictRuleDBConnector(std::string path, int mode=SQLite::OPEN_READONLY): DBConnector(path, mode) {};

    bool Insert(const ConflictRule& rule);
    bool Update(const ConflictRule& rule);
    bool Delete(int id);
    void SelectAll(std::vector<ConflictRule>& rules);

    bool SwapConflictRule(const ConflictRule& rule1, const ConflictRule& rule2);
};

#endif
