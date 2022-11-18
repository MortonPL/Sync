#pragma once
#include <string>
#include <uuid/uuid.h>

class Configuration
{
public:
    Configuration();
    Configuration(int id, std::string name, uuid_t uuid, std::string pathA, std::string pathB);
    Configuration(int id, std::string name, uuid_t uuid, 
                  std::string pathA, std::string pathAaddress, std::string pathAuser,
                  std::string pathB, std::string pathBaddress, std::string pathBuser);
    Configuration(int id, std::string name, std::string uuid, int isRemote,
                  std::string pathA, std::string pathAaddress, std::string pathAuser,
                  std::string pathB, std::string pathBaddress, std::string pathBuser);
    ~Configuration();

    bool operator==(const Configuration& other)
    {
        return this->name == other.name
            && this->uuid == other.uuid
            && this->pathA == other.pathA
            && this->pathAaddress == other.pathAaddress
            && this->pathAuser == other.pathAuser
            && this->pathB == other.pathB
            && this->pathBaddress == other.pathBaddress
            && this->pathBuser == other.pathBuser
            && this->isRemote == other.isRemote;
    }

    int id;
    std::string name;
    uuid_t uuid;
    std::string pathA;
    std::string pathAaddress;
    std::string pathAuser;
    std::string pathB;
    std::string pathBaddress;
    std::string pathBuser;
    bool isRemote;
};
