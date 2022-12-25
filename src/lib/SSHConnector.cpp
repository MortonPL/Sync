#include "Lib/SSHConnector.h"

#include "limits.h"

#include "Utils.h"

SSHConnector::SSHConnector()
{
}

SSHConnector::~SSHConnector()
{
}

bool SSHConnector::BeginSession(std::string host, std::string user)
{
    if ((session = ssh_new()) == NULL)
        return false;
    long timeout = 10;
    ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, user.c_str());
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout);
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

bool SSHConnector::AuthenticateUserPass(std::string password)
{
    return AuthenticateResult(ssh_userauth_password(session, NULL, password.c_str()));
}

int SSHConnector::GetAuthStatus()
{
    return authStatus;
}

bool SSHConnector::AuthenticateResult(int rc)
{
    switch(rc)
    {
    case SSH_AUTH_ERROR:
        authStatus = AUTH_STATUS_ERROR;
        return false;
    case SSH_AUTH_DENIED:
        return false;
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, NULL);
        authStatus = AUTH_STATUS_PARTIAL;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AUTH_STATUS_OK;
        return true;
    default:
        break;
    }

    return false;
}

bool SSHConnector::AuthenticateUserNone()
{
    switch(ssh_userauth_none(session, NULL))
    {
    case SSH_AUTH_ERROR:
        authStatus = AUTH_STATUS_ERROR;
        return false;
    case SSH_AUTH_DENIED:
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, NULL);
        authStatus = AUTH_STATUS_PARTIAL;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AUTH_STATUS_OK;
        return true;
    default:
        break;
    }

    return false;
}

bool SSHConnector::AuthenticateUserKey(int unused)
{
    return AuthenticateResult(ssh_userauth_publickey_auto(session, NULL, NULL));
}

bool SSHConnector::AuthenticateUserInteractive(std::string response)
{
    return false;
}

void printmethods(int m)
{
    std::cout << "pubkey " << (m & SSH_AUTH_METHOD_PUBLICKEY) << '\n';
    std::cout << "kbint " << (m & SSH_AUTH_METHOD_INTERACTIVE) << '\n';
    std::cout << "passwd " << (m & SSH_AUTH_METHOD_PASSWORD) << '\n';
    std::cout << "none " << (m & SSH_AUTH_METHOD_NONE) << '\n';
    std::cout << "host " << (m & SSH_AUTH_METHOD_HOSTBASED) << '\n';
    std::cout << "gsapi " << (m & SSH_AUTH_METHOD_GSSAPI_MIC) << '\n';
    std::cout << "??? " << (m & SSH_AUTH_METHOD_UNKNOWN) << '\n';
}

bool SSHConnector::AuthenticateUser(passProviderType passProvider,
                                    interactiveProviderType interactiveProvider,
                                    keyProviderType keyProvider,
                                    void* passData, void* interactiveData, void* keyData)
{
    bool isError = false;
    switch(authStatus)
    {
    case AUTH_STATUS_NONE:
        return AuthenticateUserNone();
    case AUTH_STATUS_PARTIAL:
        if (authMethods & SSH_AUTH_METHOD_PUBLICKEY)
        {
            auto res = keyProvider(isError, keyData);
            if (isError)
            {
                authStatus = SSH_AUTH_ERROR;
                return false;
            }
            return AuthenticateUserKey(res);
        }
        if (authMethods & SSH_AUTH_METHOD_PASSWORD)
        {
            auto res = passProvider(isError, passData);
            if (isError)
            {
                authStatus = SSH_AUTH_ERROR;
                return false;
            }
            return AuthenticateUserPass(res);
        }
        if (authMethods & SSH_AUTH_METHOD_INTERACTIVE)
        {
            auto res = interactiveProvider(isError, interactiveData);
            if (isError)
            {
                authStatus = SSH_AUTH_ERROR;
                return false;
            }
            return AuthenticateUserInteractive(res);
        }
        break;
    case AUTH_STATUS_OK:
        return true;
    default:
        break;
    }

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
