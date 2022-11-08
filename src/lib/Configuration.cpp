#include "../headers/Configuration.h"

Configuration::Configuration()
{
}


Configuration::Configuration(std::string name, std::string pathA, std::string pathB)
{
    this->name = name;
    this->pathA = pathA;
    this->pathB = pathB;
}

Configuration::Configuration(std::string name, std::string pathA, std::string pathB, std::string remoteAddress, std::string remoteUser)
{
    this->name = name;
    this->pathA = pathA;
    this->pathB = pathB;
    this->isRemote = true;
    this->remoteAddress = remoteAddress;
    this->remoteUser = remoteUser;
}

Configuration::~Configuration()
{
}
