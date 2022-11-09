#include "SSHConnector.h"

#include <fmt/core.h>
#include "Logger.h"

SSHConnector::SSHConnector()
{
}

SSHConnector::~SSHConnector()
{
}

bool SSHConnector::BeginSession(std::string host)
{
    this->session = ssh_new();
    if (this->session == NULL)
        return false;
    ssh_options_set(this->session, SSH_OPTIONS_HOST, host.c_str());

    return ssh_connect(session) == SSH_OK;
}

void SSHConnector::EndSession()
{
    ssh_disconnect(this->session);
    ssh_free(this->session);
}

bool SSHConnector::AuthenticateServer()
{
    auto state = ssh_session_is_known_server(this->session);
    switch(state)
    {
        case SSH_KNOWN_HOSTS_OK:
            break;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
        case SSH_KNOWN_HOSTS_UNKNOWN:
            ssh_session_update_known_hosts(this->session);
            break;
        case SSH_KNOWN_HOSTS_OTHER:
            // warn
            break;
        case SSH_KNOWN_HOSTS_CHANGED:
            // warn !!!
            break;
        case SSH_KNOWN_HOSTS_ERROR:
            // error
            break;
    }
    return true;
}

bool SSHConnector::AuthenticateUserPass(std::string user, std::string password)
{
    return ssh_userauth_password(this->session, user.c_str(), password.c_str()) == SSH_AUTH_SUCCESS;
}

ssh_channel_struct* SSHConnector::GetChannel()
{
    auto pChannel = ssh_channel_new(this->session);
    if (pChannel == NULL)
        return NULL;
    if (ssh_channel_open_session(pChannel) != SSH_OK)
        return NULL;
    return pChannel;
}

void SSHConnector::FreeChannel(ssh_channel_struct* pChannel)
{
    ssh_channel_send_eof(pChannel);
    ssh_channel_close(pChannel);
    ssh_channel_free(pChannel);
}

bool SSHConnector::ExecuteLS(std::string &result)
{
    auto pChannel = GetChannel();

    if (ssh_channel_request_exec(pChannel, "ls") != SSH_OK)
    {
        FreeChannel(pChannel);
        return false;
    }
    
    // read response
    char buffer[256];
    auto nbytes = ssh_channel_read(pChannel, buffer, sizeof(buffer), 0);
    if (nbytes < 0)
    {
        FreeChannel(pChannel);
        return false;
    }
    while (nbytes > 0)
    {
        if (nbytes <= 255)
            buffer[nbytes] = 0;
        result.append(buffer);
        nbytes = ssh_channel_read(pChannel, buffer, sizeof(buffer), 0);
    }

    FreeChannel(pChannel);
    return true;
}

// TODO: replace with executing remote Sync CLI that uses stat()
bool SSHConnector::ExecuteCD(std::string directory)
{
    std::string cmd = fmt::format("[ ! -d \"{}\"] && echo 1", directory);
    auto pChannel = GetChannel();
    char buf;

    if (ssh_channel_request_exec(pChannel, cmd.c_str()) != SSH_OK)
    {
        FreeChannel(pChannel);
        return false;
    }

    if (ssh_channel_read(pChannel, &buf, 1, 0) > 0)
    {
        FreeChannel(pChannel);
        return false;
    }

    FreeChannel(pChannel);
    return true;
}
