#include "Domain/Configuration.h"

#include <Utils.h>

Configuration::Configuration()
{
}

Configuration::Configuration(int id, std::string name, uuid_t uuid, std::string pathA,
                             std::string pathB, std::string pathBaddress, std::string pathBuser)
{
    this->id = id;
    this->name = name;
    uuid_copy(this->uuid, uuid);
    this->pathA = pathA;
    this->pathB = pathB;
    this->pathBaddress = pathBaddress;
    this->pathBuser = pathBuser;
}

Configuration::Configuration(int id, std::string name, std::string uuid, std::string timestamp, std::string pathA,
                             std::string pathB, std::string pathBaddress, std::string pathBuser)
{
    this->id = id;
    this->name = name;
    uuid_parse(uuid.c_str(), this->uuid);
    this->pathA = pathA;
    this->pathB = pathB;
    this->pathBaddress = pathBaddress;
    this->pathBuser = pathBuser;
    this->timestamp = Utils::StringToTimestamp(timestamp);
}

Configuration::Configuration(int id, std::string name, uuid_t uuid, time_t timestamp, std::string pathA,
                             std::string pathB, std::string pathBaddress, std::string pathBuser, int __unused)
{
    this->id = id;
    this->name = name;
    uuid_copy(this->uuid, uuid);
    this->pathA = pathA;
    this->pathB = pathB;
    this->pathBaddress = pathBaddress;
    this->pathBuser = pathBuser;
    this->timestamp = timestamp;
}

Configuration::~Configuration()
{
}
