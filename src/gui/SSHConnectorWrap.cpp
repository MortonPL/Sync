#include "GUI/SSHConnectorWrap.h"

#include <uuid/uuid.h>

#include "GUI/GenericPopup.h"
#include "Lib/Global.h"
#include "Utils.h"

bool initializeSession(SSHConnector& ssh, std::string address, std::string user)
{
    if (!ssh.BeginSession(address, user))
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
    return true;
}

bool showError(SSHConnector& ssh)
{
    GenericPopup("Failed to connect.").ShowModal();
    ssh.EndSession();
    return false;
}

std::string passwordProvider(bool& isError, void* data)
{
    std::string password;
    isError = GenericPopup(*(std::string*)(data),NULL, &password, true, true).ShowModal() != wxID_OK;
    return password;
};

int keyProvider(bool& isError, void* data)
{
    isError = false;
    return 0;
}

bool SSHConnectorWrap::Connect(SSHConnector& ssh, const std::string& address, const std::string& user)
{
    if (!initializeSession(ssh, address, user))
        return false;

    std::string passPrompt = fmt::format("Enter password for {}@{}:", user, address);
    void* passData = &passPrompt;
    void* interactiveData;
    void* keyData;

    while(ssh.GetAuthStatus() != AUTH_STATUS_OK)
    {
        while (!ssh.AuthenticateUser(passwordProvider, passwordProvider, keyProvider,
                                     passData, interactiveData, keyData)
               && ssh.GetAuthStatus() != AUTH_STATUS_ERROR)
        {
        }

        if (ssh.GetAuthStatus() == AUTH_STATUS_ERROR)
            return showError(ssh);
    }

    return true;
}
