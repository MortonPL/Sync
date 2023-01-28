#include "Lib/DBConnector.h"

#include <uuid/uuid.h>

std::string DBConnectorStatic::GetMainFileName()
{
    return "sync.db3";
}

bool DBConnectorStatic::EnsureCreatedMain()
{
    try
    {
        SQLite::Database db(Utils::GetDatabasePath() + GetMainFileName(), SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);

        // create tables
        db.exec(
            "CREATE TABLE IF NOT EXISTS configs ("
            "id INTEGER PRIMARY KEY,"
            "name TEXT UNIQUE NOT NULL,"
            "uuid BLOB NOT NULL,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "root_A TEXT NOT NULL,"
            "root_B TEXT NOT NULL,"
            "root_B_address TEXT NOT NULL,"
            "root_B_user TEXT NOT NULL)"
        );
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return false;
    };

    return true;
}

bool DBConnectorStatic::EnsureCreatedHistory(const std::string path)
{
    try
    {
        SQLite::Database db(Utils::GetDatabasePath() + path, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);

        // create tables
        db.exec(
            "CREATE TABLE IF NOT EXISTS header ("
            "id INTEGER PRIMARY KEY)"
        );
        db.exec(
            "CREATE TABLE IF NOT EXISTS conflict_rules ("
            "id INTEGER PRIMARY KEY,"
            "order_ INTEGER NOT NULL,"
            "name TEXT NOT NULL,"
            "rule TEXT NOT NULL,"
            "command TEXT NOT NULL)"
        );
        db.exec(
            "CREATE TABLE IF NOT EXISTS nodes ("
            "path TEXT PRIMARY KEY,"
            "dev INTEGER NOT NULL,"
            "inode INTEGER NOT NULL,"
            "r_dev INTEGER NOT NULL,"
            "r_inode INTEGER NOT NULL,"
            "mtime INTEGER NOT NULL,"
            "r_mtime INTEGER NOT NULL,"
            "size INTEGER NOT NULL,"
            "hash_high BLOB NOT NULL,"
            "hash_low BLOB NOT NULL)"
        );

        SQLite::Statement query(db, "SELECT * from nodes");
        return query.executeStep()? true: query.hasRow();
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to open history database " << path << " Reason: " << e.what();
        return false;
    };
}

void ConfigurationDBConnector::Insert(const Configuration& config)
{
    const int uuidStringSize = 36;
    char uuidstr[uuidStringSize+1];
    uuid_unparse(config.uuid, uuidstr);
    try
    {
        this->db.exec(fmt::format(
            "INSERT INTO configs "
            "(name, uuid, root_A, root_B, root_B_address, root_B_user) "
            "VALUES (\"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\")",
            config.name, uuidstr, config.pathA, config.pathB, config.pathBaddress, config.pathBuser));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to insert config. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void ConfigurationDBConnector::Update(const Configuration& config)
{
    try
    {
        this->db.exec(fmt::format(
            "UPDATE configs SET "
            "name = \"{}\", timestamp = CURRENT_TIMESTAMP,"
            "root_A = \"{}\", root_B = \"{}\", root_B_address = \"{}\", root_B_user = \"{}\""
            "WHERE id = {}",
            config.name, config.pathA, config.pathB, config.pathBaddress, config.pathBuser, config.id));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to update config. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void ConfigurationDBConnector::Delete(const int id)
{
    try
    {
        this->db.exec(fmt::format("DELETE FROM configs WHERE id = {}", id));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to delete config. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void ConfigurationDBConnector::SelectAll(std::vector<Configuration>& configs)
{
    try
    {
        SQLite::Statement query(this->db, "SELECT * from configs");
        while(query.executeStep())
        {
            configs.push_back(query.getColumns<Configuration, 8>());
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to select configs. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

bool ConfigurationDBConnector::SelectByUUID(const uuid_t& uuid, Configuration& config)
{
    const int uuidStringSize = 36;
    char uuidstr[uuidStringSize+1];
    uuid_unparse(uuid, uuidstr);
    try
    {
        SQLite::Statement query(this->db, fmt::format("SELECT * from configs WHERE uuid=\"{}\"", uuidstr));
        if (query.executeStep())
            config = query.getColumns<Configuration, 8>();
        else
            return false;
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to select config. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
    return true;
}

void HistoryFileNodeDBConnector::Insert(const HistoryFileNode& file)
{
    try
    {
        SQLite::Statement query(db, fmt::format(
            "INSERT INTO nodes "
            "(path, dev, inode, r_dev, r_inode, mtime, r_mtime, size, hash_high, hash_low) "
            "VALUES (\"{}\", {}, {}, {}, {}, {}, {}, {}, ?, ?)",
            file.path, file.dev, file.inode, file.remoteDev, file.remoteInode, file.mtime, file.remoteMtime, file.size));
        query.bind(1, &file.hashHigh, 8);
        query.bind(2, &file.hashLow, 8);
        query.exec();
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to insert file node. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void HistoryFileNodeDBConnector::Update(const HistoryFileNode& file)
{
    try
    {
        SQLite::Statement query(db, fmt::format(
            "REPLACE INTO nodes "
            "(path, dev, inode, r_dev, r_inode, mtime, r_mtime, size, hash_high, hash_low) "
            "VALUES (\"{}\", {}, {}, {}, {}, {}, {}, {}, ?, ?)",
            file.path, file.dev, file.inode, file.remoteDev, file.remoteInode, file.mtime, file.remoteMtime, file.size));
        query.bind(1, &file.hashHigh, 8);
        query.bind(2, &file.hashLow, 8);
        query.exec();
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to update file node. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void HistoryFileNodeDBConnector::Delete(const std::string path)
{
    try
    {
        this->db.exec(fmt::format("DELETE FROM nodes WHERE path = \"{}\"", path));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to delete file node. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void HistoryFileNodeDBConnector::SelectAll(std::forward_list<HistoryFileNode>& nodes)
{
    try
    {
        SQLite::Statement query(this->db, "SELECT * from nodes");
        while(query.executeStep())
        {
            nodes.push_front(HistoryFileNode
            (
                (std::string)query.getColumn(0),                // path
                (uint32_t)query.getColumn(1),                   // dev
                (int64_t)query.getColumn(2),                    // inode
                (uint32_t)query.getColumn(3),                   // r_dev
                (int64_t)query.getColumn(4),                    // r_inode
                (uint32_t)query.getColumn(5),                   // mtime
                (uint32_t)query.getColumn(6),                   // r_mtime
                (uint32_t)query.getColumn(7),                   // size
                *(XXH64_hash_t*)query.getColumn(8).getBlob(),   // hash_high
                *(XXH64_hash_t*)query.getColumn(9).getBlob()    // hash_low
            ));
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to select file nodes. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void ConflictRuleDBConnector::Insert(const ConflictRule& rule)
{
    try
    {
        this->db.exec(fmt::format(
            "INSERT INTO conflict_rules "
            "(name, rule, command, order_) "
            "VALUES (\"{}\", \"{}\", \"{}\","
            "(SELECT IFNULL(MAX(order_), 0) + 1 FROM conflict_rules)"
            ")",
            rule.name, rule.rule, rule.command));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to insert conflict rule. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void ConflictRuleDBConnector::Update(const ConflictRule& rule)
{
    try
    {
        this->db.exec(fmt::format(
            "UPDATE conflict_rules SET "
            "name = \"{}\", rule = \"{}\", command = \"{}\" "
            "WHERE id = {}",
            rule.name, rule.rule, rule.command, rule.id));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to update conflict rule. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void ConflictRuleDBConnector::Delete(const int id)
{
    try
    {
        this->db.exec(fmt::format("DELETE FROM conflict_rules WHERE id = {}", id));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to delete conflict rule. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void ConflictRuleDBConnector::SwapConflictRule(const ConflictRule& rule1, const ConflictRule& rule2)
{
    try
    {
        this->db.exec(fmt::format(
            "UPDATE conflict_rules SET order_ = "
            "CASE id WHEN {0} THEN {3} WHEN {1} THEN {2} END "
            "WHERE id IN ({0}, {1})",
            rule1.id, rule2.id, rule1.order, rule2.order));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to swap conflict rules. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}

void ConflictRuleDBConnector::SelectAll(std::vector<ConflictRule>& nodes)
{
    try
    {
        SQLite::Statement query(this->db, "SELECT * from conflict_rules ORDER BY order_");
        while(query.executeStep())
        {
            nodes.push_back(query.getColumns<ConflictRule, 5>());
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to select conflict rules. Reason: " << e.what();
        throw DBConnectorStatic::DBException();
    }
}
