#ifndef SRC_DOMAIN_CONFIGURATION_H
#define SRC_DOMAIN_CONFIGURATION_H
#include <string>
#include <uuid/uuid.h>

/*A domain class representing an app config.*/
class Configuration
{
public:
    Configuration();
    Configuration(const int id, const std::string name, const uuid_t uuid, const std::string pathA,
                  const std::string pathB, const std::string pathBaddress, const std::string pathBuser);  // New Config CTOR
    Configuration(const int id, const std::string name, const std::string uuid, const std::string timestamp, const std::string pathA,
                  const std::string pathB, const std::string pathBaddress, const std::string pathBuser);  // DB Select CTOR
    Configuration(const int id, const std::string name, const uuid_t uuid, const time_t timestamp, const std::string pathA,
                  const std::string pathB, const std::string pathBaddress, const std::string pathBuser, const int __unused);  // Full CTOR
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
