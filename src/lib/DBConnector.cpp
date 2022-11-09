#include "DBConnector.h"

#include <fmt/core.h>
#include "Logger.h"
#include "Utils.h"

std::string DBConnector::Filename;

DBConnector::DBConnector()
{
}

DBConnector::~DBConnector()
{
}

bool DBConnector::EnsureCreated()
{
    DBConnector::Filename = Utils::GetProgramPath() + "Sync.db3";
    try
    {
        SQLite::Database db(DBConnector::Filename, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);

        // create tables
        db.exec(
"CREATE TABLE IF NOT EXISTS configs (\
id INTEGER PRIMARY KEY,\
name TEXT UNIQUE NOT NULL,\
root_A TEXT NOT NULL,\
root_B TEXT NOT NULL,\
is_remote BOOLEAN NOT NULL,\
remote_address TEXT,\
remote_user TEXT\
)");
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        return false;
    };

    return true;
}

bool DBConnector::Open(int mode)
{
    this->db = new SQLite::Database(DBConnector::Filename, mode);
    return true;
}

bool DBConnector::Close()
{
    delete this->db;
    return true;
}

bool DBConnector::InsertConfig(Configuration config)
{
    try
    {
        if (config.isRemote)
        {
            this->db->exec(fmt::format("INSERT INTO configs (name, root_A, root_B, is_remote, remote_address, remote_user) VALUES (\"{}\", \"{}\", \"{}\", TRUE, \"{}\", \"{}\")",
                config.name, config.pathA, config.pathB, config.remoteAddress, config.remoteUser));
        }
        else
        {
            this->db->exec(fmt::format("INSERT INTO configs (name, root_A, root_B, is_remote) VALUES (\"{}\", \"{}\", \"{}\", FALSE)",
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

Configuration DBConnector::SelectConfig()
{

}
