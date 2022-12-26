#pragma once
#include <libssh/libssh.h>
#include <string>
#include <vector>

#include "Domain/FileNode.h"

#define AUTH_STATUS_ERROR       -1
#define AUTH_STATUS_NONE        0
#define AUTH_STATUS_PARTIAL     1
#define AUTH_STATUS_CHALLENGE   2
#define AUTH_STATUS_OK          3

typedef std::string (*passProviderType)(bool& isCanceled, std::string& prompt);
typedef std::string (*interactiveProviderType)(bool& isCanceled, std::string& challenge, bool shouldBeHidden);
typedef int (*keyProviderType)(bool& isCanceled, void* data);
/*A wrapper class for managing SSH communications.*/
class SSHConnector
{
public:
    SSHConnector();
    ~SSHConnector();

    bool BeginSession(std::string host, std::string user);
    void EndSession();
    bool AuthenticateServer();
    bool AuthenticateUser(passProviderType passwordProvider,
                          interactiveProviderType interactiveProvider,
                          keyProviderType keyProvider,
                          std::string& passPrompt, void* keyData);
    int GetAuthStatus();
    bool IsAuthDenied();

    bool CreateTunnels();
    int CallCLITest(std::string dirToCheck);
    std::vector<FileNode> CallCLICreep(std::string dirToCreep);

private:
    ssh_channel_struct* GetChannel();
    void FreeChannel(ssh_channel_struct* pChannel);
    ssh_channel_struct* CallCLI(std::string cmd);
    ssh_channel_struct* PrepareReverseTunnel();

    bool AuthenticateUserNone();
    bool AuthenticateUserKey(int unused);
    bool AuthenticateUserInteractiveFetch(std::string& name, std::string& instruction, int& nprompts);
    bool AuthenticateUserInteractive(interactiveProviderType, std::string& name, std::string& instruction, int nprompts);
    bool AuthenticateUserPass(std::string password);
    bool AuthenticateResult(int rc);

    ssh_session session;
    int authStatus = AUTH_STATUS_NONE;
    int authMethods = 0;
    bool isAuthDenied = false;
};
