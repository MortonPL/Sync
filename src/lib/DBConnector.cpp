#include "../headers/DBConnector.h"

#include <fmt/core.h>
#include "../headers/Logger.h"

DBConnector::DBConnector()
{
}

DBConnector::~DBConnector()
{
}

bool DBConnector::EnsureCreated()
{
    try
    {
        SQLite::Database db = SQLite::Database(DATABASE_NAME, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);

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
        return false;
    };

    return true;
}

bool DBConnector::Open(int mode)
{
    this->db = new SQLite::Database(DATABASE_NAME, mode);
    return true;
}

bool DBConnector::Close()
{
    delete this->db;
    return true;
}

bool DBConnector::InsertConfig(Configuration config)
{
    return true;
}

Configuration DBConnector::SelectConfig()
{

}
