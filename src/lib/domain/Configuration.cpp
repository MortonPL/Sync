#include "Domain/Configuration.h"

#include <Utils.h>

Configuration::Configuration()
{
}

Configuration::Configuration(const int id, const std::string name, const uuid_t uuid, const std::string pathA,
                             const std::string pathB, const std::string pathBaddress, const std::string pathBuser)
{
    this->id = id;
    this->name = name;
    uuid_copy(this->uuid, uuid);
    this->pathA = pathA;
    this->pathB = pathB;
    this->pathBaddress = pathBaddress;
    this->pathBuser = pathBuser;
}

Configuration::Configuration(const int id, const std::string name, const std::string uuid, const std::string timestamp, const std::string pathA,
                             const std::string pathB, const std::string pathBaddress, const std::string pathBuser)
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

Configuration::Configuration(const int id, const std::string name, const uuid_t uuid, const time_t timestamp, const std::string pathA,
                             const std::string pathB, const std::string pathBaddress, const std::string pathBuser, int)
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
