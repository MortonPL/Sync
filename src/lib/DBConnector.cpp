#include "Lib/DBConnector.h"

#include <fmt/core.h>
#include "Utils/Logger.h"
#include "Utils/Utils.h"

std::string DBConnector::filename;

DBConnector::DBConnector(int mode): db(DBConnector::filename, mode)
{
}

DBConnector::~DBConnector()
{
}

// run this method at the start of the program!
bool DBConnector::EnsureCreated()
{
    DBConnector::filename = Utils::GetProgramPath() + "Sync.db3";
    try
    {
        SQLite::Database db(DBConnector::filename, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);

        // create tables
        db.exec(
            "CREATE TABLE IF NOT EXISTS configs ("
            "id INTEGER PRIMARY KEY,"
            "name TEXT UNIQUE NOT NULL,"
            "root_A TEXT NOT NULL,"
            "root_B TEXT NOT NULL,"
            "is_remote BOOLEAN NOT NULL,"
            "remote_address TEXT,"
            "remote_user TEXT)"
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
    try
    {
        if (config.isRemote)
        {
            this->db.exec(fmt::format("INSERT INTO configs (name, root_A, root_B, is_remote, remote_address, remote_user) VALUES (\"{}\", \"{}\", \"{}\", TRUE, \"{}\", \"{}\")",
                config.name, config.pathA, config.pathB, config.remoteAddress, config.remoteUser));
        }
        else
        {
            this->db.exec(fmt::format("INSERT INTO configs (name, root_A, root_B, is_remote) VALUES (\"{}\", \"{}\", \"{}\", FALSE)",
                config.name, config.pathA, config.pathB));
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
        this->db.exec(fmt::format("UPDATE configs SET name = \"{}\", root_A = \"{}\", root_b = \"{}\", is_remote = {}, remote_address = \"{}\", remote_user = \"{}\" WHERE id = {}",
            config.name, config.pathA, config.pathB, config.isRemote ? "TRUE" : "FALSE", config.remoteAddress, config.remoteUser, config.id));
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
        configs.push_back(query.getColumns<Configuration, 7>());
    }
    return configs;
}
