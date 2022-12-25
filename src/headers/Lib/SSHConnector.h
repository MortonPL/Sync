#pragma once
#include <libssh/libssh.h>
#include <string>
#include <vector>

#include "Domain/FileNode.h"

#define AUTH_STATUS_ERROR   -1
#define AUTH_STATUS_NONE    0
#define AUTH_STATUS_PARTIAL 1
#define AUTH_STATUS_LIST    2
#define AUTH_STATUS_OK      10

typedef std::string (*passProviderType)(bool& isError, void* data);
typedef std::string (*interactiveProviderType)(bool& isError, void* data);
typedef int (*keyProviderType)(bool& isError, void* data);
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
                          void* passData, void* interactiveData, void* keyData);
    int GetAuthStatus();
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
    bool AuthenticateUserInteractive(std::string response);
    bool AuthenticateUserPass(std::string password);
    bool AuthenticateResult(int rc);

    ssh_session session;
    int authStatus = AUTH_STATUS_NONE;
    int authMethods = 0;
};
