#pragma once
#include <string>

class Configuration
{
public:
    Configuration();
    Configuration(int id, std::string name, std::string pathA, std::string pathB);
    Configuration(int id, std::string name, std::string pathA, std::string pathB, std::string remoteAddress, std::string remoteUser);
    Configuration(int id, std::string name, std::string pathA, std::string pathB, int isRemote, std::string remoteAddress, std::string remoteUser);
    ~Configuration();

    bool operator==(const Configuration& other)
    {
        return this->name == other.name
            && this->pathA == other.pathA
            && this->pathB == other.pathB
            && this->isRemote == other.isRemote
            && this->remoteAddress == other.remoteUser;
    }

    int id;
    std::string name;
    std::string pathA;
    std::string pathB;
    bool isRemote;
    std::string remoteAddress;
    std::string remoteUser;
};
