#include "Lib/DBConnector.h"

#include <uuid/uuid.h>
#include "Utils.h"

std::string DBConnector::mainFile;

DBConnector::DBConnector(std::string filename, int mode): db(filename, mode)
{
}

DBConnector::~DBConnector()
{
}

std::string& DBConnector::GetMainFileName()
{
    return DBConnector::mainFile;
}

// run this method at the start of the program!
bool DBConnector::EnsureCreated()
{
    DBConnector::mainFile = Utils::GetDatabasePath() + "sync.db3";
    try
    {
        SQLite::Database db(DBConnector::mainFile, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);

        // create tables
        db.exec(
            "CREATE TABLE IF NOT EXISTS configs ("
            "id INTEGER PRIMARY KEY,"
            "name TEXT UNIQUE NOT NULL,"
            "uuid BLOB NOT NULL,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "is_remote BOOLEAN NOT NULL,"
            "root_A TEXT NOT NULL,"
            "root_A_address TEXT,"
            "root_A_user TEXT,"
            "root_B TEXT NOT NULL,"
            "root_B_address TEXT,"
            "root_B_user TEXT)"
        );
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return false;
    };

    return true;
}

bool DBConnector::InsertConfig(Configuration config)
{
    char uuidstr[36+1];
    uuid_unparse(config.uuid, uuidstr);
    try
    {
        if (config.isRemote)
        {
            this->db.exec(fmt::format(
                "INSERT INTO configs "
                "(name, uuid, is_remote, root_A, root_A_address, root_A_user, root_B, root_B_address, root_B_user) "
                "VALUES (\"{}\", \"{}\", TRUE, \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\")",
                config.name, uuidstr, config.pathA, config.pathAaddress,
                config.pathAuser, config.pathB, config.pathBaddress, config.pathBuser));
        }
        else
        {
            this->db.exec(fmt::format(
                "INSERT INTO configs "
                "(name, uuid, is_remote, root_A, root_B) "
                "VALUES (\"{}\", \"{}\", FALSE, \"{}\", \"{}\")",
                config.name, uuidstr, config.pathA, config.pathB));
        }
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
            "name = \"{}\", is_remote = {}, timestamp = CURRENT_TIMESTAMP,"
            "root_A = \"{}\", root_A_address = \"{}\", root_A_user = \"{}\","
            "root_B = \"{}\", root_B_address = \"{}\", root_B_user = \"{}\""
            "WHERE id = {}",
            config.name, config.isRemote ? "TRUE" : "FALSE",
            config.pathA, config.pathAaddress, config.pathAuser,
            config.pathB, config.pathBaddress, config.pathBuser, config.id));
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
        configs.push_back(query.getColumns<Configuration, 11>());
    }
    return configs;
}

Configuration DBConnector::SelectConfigByUUID(std::string uuid)
{
    SQLite::Statement query(this->db, fmt::format("SELECT * from configs WHERE uuid = {}", uuid));
    if (!query.executeStep())
        return Configuration();
    return query.getColumns<Configuration, 11>();
}
