#pragma once
#include <libssh/libssh.h>
#include <string>
#include <forward_list>

#include "Domain/FileNode.h"

#define AUTH_STATUS_ERROR       -1
#define AUTH_STATUS_NONE        0
#define AUTH_STATUS_UNHANDLED   1
#define AUTH_STATUS_PARTIAL     2
#define AUTH_STATUS_CHALLENGE   3
#define AUTH_STATUS_OK          4

#define CALLCLI_OK 0
#define CALLCLI_ERROR -1
#define CALLCLI_404 -2
#define CALLCLI_NOANSWER -3

typedef void (*genericMessengerType)(std::string prompt);
typedef std::string (*passProviderType)(bool& isCanceled, std::string& prompt);
typedef std::string (*interactiveProviderType)(bool& isCanceled, std::string& challenge, bool shouldBeHidden);
typedef std::string (*keyProviderType)(bool& isCanceled, std::string& prompt);
typedef bool (*serverHashCallbackType)(std::string& pubkeyHash);

/*A class for managing SSH communications.*/
class SSHConnector
{
public:
    SSHConnector();
    ~SSHConnector();

    bool Connect(const std::string& address, const std::string& user,
                 genericMessengerType genericMessenger, serverHashCallbackType unknownCallback,
                 serverHashCallbackType otherCallback, serverHashCallbackType changedCallback,
                 serverHashCallbackType errorCallback, passProviderType passwordProvider,
                 interactiveProviderType interactiveProvider, keyProviderType keyProvider);
    void EndSession();
    bool IsActiveSession();

    int CallCLICreep(std::string dirToCreep, std::forward_list<FileNode>& nodes);

private:
    ssh_channel_struct* GetChannel();
    void FreeChannel(ssh_channel_struct* pChannel);
    ssh_channel_struct* CallCLI(std::string cmd);
    ssh_channel_struct* PrepareReverseTunnel();
    bool CreateTunnels();

    bool BeginSession(std::string host, std::string user);
    bool AuthenticateServer(serverHashCallbackType unknownCallback,
                            serverHashCallbackType otherCallback,
                            serverHashCallbackType changedCallback,
                            serverHashCallbackType errorCallback);
    bool AuthenticateUser(passProviderType passwordProvider,
                          interactiveProviderType interactiveProvider,
                          keyProviderType keyProvider,
                          std::string& passPrompt, std::string& keyPrompt);
    bool AuthenticateUserNone();
    bool AuthenticateGSSAPI();
    bool AuthenticateUserKeyAuto();
    bool AuthenticateUserKeyFetch(keyProviderType provider, std::string keyPrompt, ssh_key* pPrivate);
    bool AuthenticateUserKey(ssh_key* pPrivate);
    bool AuthenticateUserInteractiveFetch(std::string& name, std::string& instruction, int& nprompts);
    bool AuthenticateUserInteractive(interactiveProviderType provider, std::string& name, std::string& instruction, int nprompts);
    bool AuthenticateUserPass(std::string password);
    bool AuthenticateResult(int rc);

    ssh_session session = NULL;
    int authStatus = AUTH_STATUS_NONE;
    int authMethods = 0;
    bool isAuthDenied = false;
    int retryCount = 0;
};
