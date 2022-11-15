#include "Domain/Configuration.h"

Configuration::Configuration()
{
}


Configuration::Configuration(int id, std::string name, std::string pathA, std::string pathB)
{
    this->id = id;
    this->name = name;
    this->pathA = pathA;
    this->pathB = pathB;
    this->isRemote = false;
}

Configuration::Configuration(int id, std::string name, std::string pathA, std::string pathB, std::string remoteAddress, std::string remoteUser)
{
    this->id = id;
    this->name = name;
    this->pathA = pathA;
    this->pathB = pathB;
    this->isRemote = true;
    this->remoteAddress = remoteAddress;
    this->remoteUser = remoteUser;
}

Configuration::Configuration(int id, std::string name, std::string pathA, std::string pathB, int isRemote, std::string remoteAddress, std::string remoteUser)
{
    this->id = id;
    this->name = name;
    this->pathA = pathA;
    this->pathB = pathB;
    this->isRemote = isRemote;
    this->remoteAddress = remoteAddress;
    this->remoteUser = remoteUser;
}

Configuration::~Configuration()
{
}
