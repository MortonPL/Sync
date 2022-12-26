#pragma once
#include "Lib/SSHConnector.h"

namespace SSHConnectorWrap
{
    bool Connect(SSHConnector& ssh, const std::string& address, const std::string& user);
};
