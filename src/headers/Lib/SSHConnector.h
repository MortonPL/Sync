#pragma once
#include <libssh/libssh.h>
#include <string>
#include <vector>

#include "Domain/FileNode.h"

class SSHConnector
{
public:
    SSHConnector();
    ~SSHConnector();

    bool BeginSession(std::string host);
    void EndSession();
    bool AuthenticateServer();
    bool AuthenticateUserPass(std::string name, std::string password);
    bool AuthenticateUserKey();
    bool CreateTunnels();
    int CallCLITest(std::string dirToCheck);
    std::vector<FileNode> CallCLICreep(std::string dirToCreep);

private:
    ssh_channel_struct* GetChannel();
    void FreeChannel(ssh_channel_struct* pChannel);
    ssh_channel_struct* CallCLI(std::string cmd);
    ssh_channel_struct* PrepareReverseTunnel();

    ssh_session session;
};
