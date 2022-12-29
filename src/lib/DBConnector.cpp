#include "Lib/DBConnector.h"

#include <uuid/uuid.h>
#include "Utils.h"

#define MAIN_DB_FILE "sync.db3"

DBConnector::DBConnector(std::string path, int mode): db(Utils::GetDatabasePath() + path, mode)
{
}

DBConnector::~DBConnector()
{
}

std::string DBConnector::GetMainFileName()
{
    return MAIN_DB_FILE;
}

// run this method ONCE at the start of the program!
int DBConnector::EnsureCreatedMain()
{
    try
    {
        SQLite::Database db(Utils::GetDatabasePath() + MAIN_DB_FILE, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);

        // create tables
        db.exec(
            "CREATE TABLE IF NOT EXISTS configs ("
            "id INTEGER PRIMARY KEY,"
            "name TEXT UNIQUE NOT NULL,"
            "uuid BLOB NOT NULL,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "root_A TEXT NOT NULL,"
            "root_B TEXT NOT NULL,"
            "root_B_address TEXT,"
            "root_B_user TEXT)"
        );
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return DB_FAIL;
    };

    return DB_GOOD;
}

int DBConnector::EnsureCreatedHistory(std::string path)
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
            "CREATE TABLE IF NOT EXISTS nodes ("
            "path TEXT PRIMARY KEY,"
            "dev INTEGER NOT NULL,"
            "inode INTEGER NOT NULL,"
            "mtime INTEGER NOT NULL,"
            "size INTEGER NOT NULL,"
            "hash_high BLOB NOT NULL,"
            "hash_low BLOB NOT NULL)"
        );

        SQLite::Statement query(db, "SELECT * from nodes");
        if (query.executeStep())
        {
            return DB_GOOD;
        }
        else
        {
            return query.hasRow()? DB_GOOD: DB_EMPTY;
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return DB_FAIL;
    };
}

bool DBConnector::InsertConfig(Configuration config)
{
    char uuidstr[36+1];
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
        LOG(ERROR) << e.what();
        return false;
    }

    return true;
}

bool DBConnector::UpdateConfig(Configuration config)
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
        LOG(ERROR) << e.what();
        return false;
    }

    return true;
}

bool DBConnector::DeleteConfig(int id)
{
    try
    {
        this->db.exec(fmt::format("DELETE FROM configs WHERE id = {}", id));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return false;
    }

    return true;
}

std::vector<Configuration> DBConnector::SelectAllConfigs()
{
    std::vector<Configuration> configs;
    SQLite::Statement query(this->db, "SELECT * from configs");
    while(query.executeStep())
    {
        configs.push_back(query.getColumns<Configuration, 8>());
    }
    return configs;
}

Configuration DBConnector::SelectConfigByUUID(std::string uuid)
{
    SQLite::Statement query(this->db, fmt::format("SELECT * from configs WHERE uuid = {}", uuid));
    if (!query.executeStep())
        return Configuration();
    return query.getColumns<Configuration, 8>();
}

bool DBConnector::InsertFileNode(FileNode file)
{
    try
    {
        SQLite::Statement query(db, fmt::format(
            "INSERT INTO nodes "
            "(path, dev, inode, mtime, size, hash_high, hash_low) "
            "VALUES (\"{}\", {}, {}, {}, {}, ?, ?)",
            file.path, file.dev, file.inode, file.mtime, file.size));
        query.bind(1, &file.hashHigh, 8);
        query.bind(2, &file.hashLow, 8);
        query.exec();
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return false;
    }

    return true;
}

bool DBConnector::UpdateFileNode(FileNode file)
{
    try
    {
        if (file.status == STATUS_MOVED)
        {
            SQLite::Statement query(db, fmt::format(
                "UPDATE nodes SET "
                "path = \"{}\", mtime = {}, size = {}, "
                "hash_high = ?, hash_low = ?"
                "WHERE dev = {} AND inode = {}",
                file.path, file.mtime, file.size, file.dev, file.inode));
            query.bind(1, &file.hashHigh, 8);
            query.bind(2, &file.hashLow, 8);
            query.exec();
        }
        else
        {
            SQLite::Statement query(db, fmt::format(
                "UPDATE nodes SET "
                "dev = {}, inode = {}, mtime = {}, size = {}, "
                "hash_high = ?, hash_low = ?"
                "WHERE path = \"{}\"",
                file.dev, file.inode, file.mtime, file.size, file.path));
            query.bind(1, &file.hashHigh, 8);
            query.bind(2, &file.hashLow, 8);
            query.exec();
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return false;
    }

    return true;
}

bool DBConnector::DeleteFileNode(std::string& path)
{
    try
    {
        this->db.exec(fmt::format("DELETE FROM nodes WHERE path = {}", path));
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return false;
    }

    return true;
}

std::vector<FileNode> DBConnector::SelectAllFileNodes()
{
    std::vector<FileNode> nodes;
    SQLite::Statement query(this->db, "SELECT * from nodes");
    while(query.executeStep())
    {
        nodes.push_back(FileNode
        (
            (std::string)query.getColumn(0),
            "",
            (uint32_t)query.getColumn(1),
            (int64_t)query.getColumn(2),
            (uint32_t)query.getColumn(3),
            (uint32_t)query.getColumn(4),
            *(XXH64_hash_t*)query.getColumn(5).getBlob(),
            *(XXH64_hash_t*)query.getColumn(6).getBlob()
        ));
    }
    return nodes;
}
