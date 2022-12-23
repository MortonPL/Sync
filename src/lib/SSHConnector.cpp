#include "Lib/SSHConnector.h"

#include "limits.h"

#include "Utils.h"

SSHConnector::SSHConnector()
{
}

SSHConnector::~SSHConnector()
{
}

bool SSHConnector::BeginSession(std::string host)
{
    session = ssh_new();
    if (session == NULL)
        return false;
    ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());

    return ssh_connect(session) == SSH_OK;
}

void SSHConnector::EndSession()
{
    ssh_disconnect(session);
    ssh_free(session);
}

bool SSHConnector::AuthenticateServer()
{
    auto state = ssh_session_is_known_server(session);
    switch(state)
    {
        case SSH_KNOWN_HOSTS_OK:
            break;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
        case SSH_KNOWN_HOSTS_UNKNOWN:
            ssh_session_update_known_hosts(session);
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
    return ssh_userauth_password(session, user.c_str(), password.c_str()) == SSH_AUTH_SUCCESS;
}

bool SSHConnector::AuthenticateUserKey()
{
    return false;
}

ssh_channel_struct* SSHConnector::GetChannel()
{
    auto pChannel = ssh_channel_new(session);
    if (pChannel == NULL)
        return nullptr;
    if (ssh_channel_open_session(pChannel) != SSH_OK)
        return nullptr;
    return pChannel;
}

void SSHConnector::FreeChannel(ssh_channel_struct* pChannel)
{
    ssh_channel_send_eof(pChannel);
    ssh_channel_close(pChannel);
    ssh_channel_free(pChannel);
}

bool SSHConnector::CreateTunnels()
{
    auto pForwardChannel = ssh_channel_new(session);
    auto pReverseChannel = ssh_channel_new(session);
    if (pForwardChannel == NULL || pReverseChannel == NULL)
        return false;

    if (ssh_channel_open_forward(pForwardChannel, "localhost", 40404, "localhost", 5555) != SSH_OK)
    {
        FreeChannel(pForwardChannel);
        FreeChannel(pReverseChannel);
        return false;
    }

    /*
    if (ssh_channel_write(pForwardChannel, "ABBA", 4) != 4)
    {
        FreeChannel(pForwardChannel);
        FreeChannel(pReverseChannel);
        return false;
    }
    */

    FreeChannel(pForwardChannel);
    FreeChannel(pReverseChannel);
    return true;
}

ssh_channel_struct* SSHConnector::CallCLI(std::string cmd)
{
    auto pChannel = GetChannel();
    if (ssh_channel_request_exec(pChannel, ("~/.sync/bin/synccli " + cmd).c_str()) != SSH_OK)
    {
            FreeChannel(pChannel);
            return nullptr;
    }
    return pChannel;
}

int SSHConnector::CallCLITest(std::string dirToCheck)
{
    char buf[1];
    auto pChannel = CallCLI(fmt::format("-t {}", dirToCheck));
    if (pChannel == nullptr)
    {
        FreeChannel(pChannel);
        return 1;
    }
    if (ssh_channel_read(pChannel, &buf, 1, 0) == 0)
    {
        FreeChannel(pChannel);
        return 1;
    }

    if (buf[0] != '0') 
        return 2;

    FreeChannel(pChannel);
    return 0;
}

ssh_channel_struct* SSHConnector::PrepareReverseTunnel()
{
    if (ssh_channel_listen_forward(session, NULL, 40405, NULL) != SSH_OK)
    {
        return nullptr;
    }
    auto pReverseChannel = ssh_channel_accept_forward(session, 60000, nullptr);
    if (pReverseChannel == NULL)
    {
        fprintf(stderr, "Error waiting for incoming connection: %s\n",
                ssh_get_error(session));
        return nullptr;
    }
    return pReverseChannel;
}

#define BUFFER_SIZE (PATH_MAX + 4*sizeof(long) + 4 + 1)
std::vector<FileNode> SSHConnector::CallCLICreep(std::string dirToCreep)
{
    std::vector<FileNode> nodes;
    char buf[BUFFER_SIZE];
    int len = 0;
    auto pChannel = CallCLI(fmt::format("-c {} -r {} {}", dirToCreep, "localhost", 40405));
    
    auto pReverseChannel = PrepareReverseTunnel();
    if (pReverseChannel == nullptr)
    {
        FreeChannel(pChannel);
        return nodes;
    }

    std::string tempString = "";
    bool incomplete = false;
    while ((len = ssh_channel_read(pReverseChannel, &buf, BUFFER_SIZE, 0)) > 0)
    {
        std::stringstream ss(buf);
        std::string path;
        while(std::getline(ss, path, '\n'))
        {
            if (!ss.fail() && !ss.eof())
            {
                if (incomplete)
                {
                    tempString += path;
                    nodes.push_back(FileNode(tempString, 0, 0, 0, 0, 0, 0));
                    tempString = "";
                    incomplete = false;
                }
                else
                {
                    nodes.push_back(FileNode(path, 0, 0, 0, 0, 0, 0));
                }
            }
            else
            {
                tempString += path;
                incomplete = true;
            }
        }
        memset(buf, 0, BUFFER_SIZE);
    }
    if (incomplete)
    {
        nodes.push_back(FileNode(tempString, 0, 0, 0, 0, 0, 0));
    }

    FreeChannel(pChannel);
    FreeChannel(pReverseChannel);
    return nodes;
}
#undef BUFFER_SIZE
