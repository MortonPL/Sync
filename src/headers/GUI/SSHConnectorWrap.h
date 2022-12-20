#pragma once
#include "Lib/SSHConnector.h"

class SSHConnectorWrap
{
public:
    static bool Connect(SSHConnector& ssh, const std::string& address, const std::string& user, const std::string pass="");
};
