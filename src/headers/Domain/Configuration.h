#ifndef SRC_DOMAIN_CONFIGURATION_H
#define SRC_DOMAIN_CONFIGURATION_H
#include <string>
#include <uuid/uuid.h>

/*A domain class representing an app config.*/
class Configuration
{
public:
    Configuration();
    Configuration(int id, std::string name, uuid_t uuid, std::string pathA,
                  std::string pathB, std::string pathBaddress, std::string pathBuser);  // New Config CTOR
    Configuration(int id, std::string name, std::string uuid, std::string timestamp, std::string pathA,
                  std::string pathB, std::string pathBaddress, std::string pathBuser);  // DB Select CTOR
    Configuration(int id, std::string name, uuid_t uuid, time_t timestamp, std::string pathA,
                  std::string pathB, std::string pathBaddress, std::string pathBuser, int __unused);  // Full CTOR
    ~Configuration();

    bool operator==(const Configuration& other) const
    {
        return this->name == other.name
            && uuid_compare(this->uuid, other.uuid) == 0
            && this->pathA == other.pathA
            && this->pathB == other.pathB
            && this->pathBaddress == other.pathBaddress
            && this->pathBuser == other.pathBuser
            && this->timestamp == other.timestamp;
    }

    int id;
    std::string name;
    uuid_t uuid;
    time_t timestamp;
    std::string pathA;
    std::string pathB;
    std::string pathBaddress;
    std::string pathBuser;
};

#endif
