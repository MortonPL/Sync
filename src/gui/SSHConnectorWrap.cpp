#include "GUI/SSHConnectorWrap.h"

#include <uuid/uuid.h>

#include "GUI/GenericPopup.h"
#include "Lib/Global.h"
#include "Utils.h"

bool SSHConnectorWrap::Connect(SSHConnector& ssh, const std::string& address, const std::string& user, const std::string pass)
{
    if (!ssh.BeginSession(address))
    {
        GenericPopup("Failed to connect to given address.").ShowModal();
        return false;
    }
    if (!ssh.AuthenticateServer())
    {
        GenericPopup("Failed to authenticate server.").ShowModal();
        ssh.EndSession();
        return false;
    }

    std::string password = pass;
    if (pass == "")
    {
        GenericPopup(
            fmt::format("Enter password for {}@{}:", user, address),
            NULL, &password, true).ShowModal();
    }
    Global::lastUsedCreds.username = user;
    Global::lastUsedCreds.password = password;

    // TODO support key auth
    if (!ssh.AuthenticateUserPass(user, password))
    {
        GenericPopup("Failed to authenticate user. Check user credentials.").ShowModal();
        ssh.EndSession();
        return false;
    }

    return true;
}
