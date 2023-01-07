#include "GUI/SSHConnectorWrap.h"

#include "GUI/GenericPopup.h"

void genericPopuper(std::string prompt)
{
    GenericPopup(prompt).ShowModal();
}

std::string passwordProvider(bool& isCanceled, std::string& prompt)
{
    std::string password;
    isCanceled = GenericPopup(prompt, NULL, &password, nullptr, GenericPopup::Flags::PasswordCancel).ShowModal() != wxID_OK;
    return password;
}

std::string interactiveProvider(bool& isCanceled, std::string& challenge, bool shouldBeHidden)
{
    std::string password;
    GenericPopup::Flags flags = GenericPopup::Flags::Cancel;
    if (shouldBeHidden)
        flags = GenericPopup::Flags::PasswordCancel;
    isCanceled = GenericPopup(challenge, NULL, &password, nullptr, flags).ShowModal() != wxID_OK;
    return password;
}

bool unknownCallback(std::string& pubkeyHash)
{
    std::string msg = "This host is unknown. Do you trust this host?\nKey:";
    return GenericPopup(msg, NULL, nullptr, &pubkeyHash, GenericPopup::Flags::Cancel).ShowModal() == wxID_OK;
}

bool otherCallback(std::string& pubkeyHash)
{
    std::string msg = "Couldn't find the key for this host, but another type of key exists.\nThis might be dangerous. Do you trust this host?\nKey: ";
    return GenericPopup(msg, NULL, nullptr, &pubkeyHash, GenericPopup::Flags::Cancel).ShowModal() == wxID_OK;
}

bool changedCallback(std::string& pubkeyHash)
{
    std::string msg = "Host key for this server has changed.\nTHIS MIGHT BE DANGEROUS. Do you trust this host?\nKey: ";
    return GenericPopup(msg, NULL, nullptr, &pubkeyHash, GenericPopup::Flags::Cancel).ShowModal() == wxID_OK;
}

bool errorCallback(std::string& pubkeyHash)
{
    pubkeyHash.size(); // unused
    return GenericPopup("An error has occured while attempting to authenticate the server.").ShowModal() == wxID_OK;
}

bool SSHConnectorWrap::Connect(SSHConnector& ssh, const std::string& address, const std::string& user)
{
    return ssh.Connect(address, user, genericPopuper,
                       unknownCallback, otherCallback, changedCallback, errorCallback,
                       passwordProvider, interactiveProvider, passwordProvider);
}
