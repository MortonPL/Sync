#ifndef SRC_GUI_SSH_CONNECTOR_WRAP_H
#define SRC_GUI_SSH_CONNECTOR_WRAP_H
#include "Lib/SSHConnector.h"

namespace SSHConnectorWrap
{
    bool Connect(SSHConnector& ssh, const std::string& address, const std::string& user);
}

#endif
