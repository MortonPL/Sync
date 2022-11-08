#pragma once
#include <string>

class Configuration
{
public:
    Configuration();
    Configuration(std::string name, std::string pathA, std::string pathB);
    Configuration(std::string name, std::string pathA, std::string pathB, std::string remoteAddress, std::string remoteUser);
    ~Configuration();

private:
    std::string name;
    std::string pathA;
    std::string pathB;
    bool isRemote;
    std::string remoteAddress;
    std::string remoteUser;
};
