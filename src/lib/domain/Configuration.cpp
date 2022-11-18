#include "Domain/Configuration.h"

Configuration::Configuration()
{
}


Configuration::Configuration(int id, std::string name, uuid_t uuid, std::string pathA, std::string pathB)
{
    this->id = id;
    this->name = name;
    uuid_copy(this->uuid, uuid);
    this->pathA = pathA;
    this->pathB = pathB;
    this->isRemote = false;
}

Configuration::Configuration(int id, std::string name, uuid_t uuid, 
                             std::string pathA, std::string pathAaddress, std::string pathAuser,
                             std::string pathB, std::string pathBaddress, std::string pathBuser)
{
    this->id = id;
    this->name = name;
    uuid_copy(this->uuid, uuid);
    this->pathA = pathA;
    this->pathAaddress = pathAaddress;
    this->pathAuser = pathAuser;
    this->pathB = pathB;
    this->pathBaddress = pathBaddress;
    this->pathBuser = pathBuser;
    this->isRemote = true;
}

Configuration::Configuration(int id, std::string name, std::string uuid, int isRemote, 
                             std::string pathA, std::string pathAaddress, std::string pathAuser,
                             std::string pathB, std::string pathBaddress, std::string pathBuser)
{
    this->id = id;
    this->name = name;
    uuid_parse(uuid.c_str(), this->uuid);
    this->pathA = pathA;
    this->pathAaddress = pathAaddress;
    this->pathAuser = pathAuser;
    this->pathB = pathB;
    this->pathBaddress = pathBaddress;
    this->pathBuser = pathBuser;
    this->isRemote = isRemote;
}

Configuration::~Configuration()
{
}
