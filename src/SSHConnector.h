#pragma once
#include <libssh/libssh.h>
#include <string>

class SSHConnector
{
public:
    SSHConnector();
    ~SSHConnector();

    bool BeginSession(std::string host);
    bool AuthenticateServer();
    bool AuthenticateUserPass(std::string name, std::string password);
    bool AuthenticateUserKey();
    bool ExecuteLS(std::string &result);
    void EndSession();

private:
    ssh_channel_struct* GetChannel();
    void FreeChannel(ssh_channel_struct* pChannel);

    ssh_session session;
};
