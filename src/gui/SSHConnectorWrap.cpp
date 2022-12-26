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

std::string passwordProvider(bool& isCanceled, std::string& prompt)
{
    std::string password;
    isCanceled = GenericPopup(prompt,NULL, &password, true, true).ShowModal() != wxID_OK;
    return password;
};

std::string interactiveProvider(bool& isCanceled, std::string& challenge, bool shouldBeHidden)
{
    std::string password;
    isCanceled = GenericPopup(challenge,NULL, &password, shouldBeHidden, true).ShowModal() != wxID_OK;
    return password;
};

int keyProvider(bool& isCanceled, void* data)
{
    isCanceled = false;
    return 0;
}

bool SSHConnectorWrap::Connect(SSHConnector& ssh, const std::string& address, const std::string& user)
{
    if (!initializeSession(ssh, address, user))
        return false;

    std::string passPrompt = fmt::format("Enter password for {}@{}:", user, address);
    void* interactiveData;
    void* keyData;

    while(ssh.GetAuthStatus() != AUTH_STATUS_OK)
    {
        while (!ssh.AuthenticateUser(passwordProvider, interactiveProvider, keyProvider,
                                     passPrompt, keyData)
               && ssh.GetAuthStatus() != AUTH_STATUS_ERROR)
        {
            if (ssh.IsAuthDenied())
                GenericPopup("Permission denied. Please try again.").ShowModal();
        }

        if (ssh.GetAuthStatus() == AUTH_STATUS_ERROR)
            return showError(ssh);
    }

    return true;
}
