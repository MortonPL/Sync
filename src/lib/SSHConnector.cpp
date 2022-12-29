#include "Lib/SSHConnector.h"

#include "limits.h"
#include <filesystem>

#include "Utils.h"

SSHConnector::SSHConnector()
{
}

SSHConnector::~SSHConnector()
{
    EndSession();
}

bool SSHConnector::Connect(const std::string& address, const std::string& user,
                   genericMessengerType genericMessenger, serverHashCallbackType unknownCallback,
                   serverHashCallbackType otherCallback, serverHashCallbackType changedCallback,
                   serverHashCallbackType errorCallback, passProviderType passwordProvider,
                   interactiveProviderType interactiveProvider, keyProviderType keyProvider)
{
    if (session != NULL)
        return false;

    if (!BeginSession(address, user))
    {
        genericMessenger("Failed to connect to given address.");
        return false;
    }
    if (!AuthenticateServer(unknownCallback, otherCallback, changedCallback, errorCallback))
    {
        genericMessenger("Failed to authenticate server.");
        EndSession();
        return false;
    }

    std::string passPrompt = fmt::format("Enter password for {}@{}:", user, address);
    std::string keyPrompt = fmt::format("Enter passphrase for private key ");

    while(authStatus != AUTH_STATUS_OK)
    {
        while (!AuthenticateUser(passwordProvider, interactiveProvider, keyProvider,
                                     passPrompt, keyPrompt)
               && (authStatus != AUTH_STATUS_ERROR && authStatus != AUTH_STATUS_UNHANDLED))
        {
            if (isAuthDenied)
                genericMessenger("Permission denied. Please try again.");
        }

        if (authStatus == AUTH_STATUS_ERROR)
        {
            genericMessenger("Failed to connect.");
            EndSession();
            return false;
        }
        else if (authStatus == AUTH_STATUS_UNHANDLED)
        {
            genericMessenger("Server requires an authentication method that is not supported (hostbased or gssapi).");
            EndSession();
            return false;
        }
    }

    return true;
}

bool SSHConnector::BeginSession(std::string host, std::string user)
{
    if ((session = ssh_new()) == NULL)
        return false;
    long timeout = 10;
    ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, user.c_str());
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout);
    if (ssh_connect(session) != SSH_OK)
    {
        ssh_free(session);
        session = NULL;
        return false;
    }
    return true;
}

void SSHConnector::EndSession()
{
    authStatus = AUTH_STATUS_NONE;
    authMethods = 0;
    isAuthDenied = false;
    retryCount = 0;
    if (session != NULL)
    {
        if (ssh_is_connected(session))
            ssh_disconnect(session);
        ssh_free(session);
        session = NULL;
    }
}

bool SSHConnector::IsActiveSession()
{
    return authStatus == AUTH_STATUS_OK;
}

bool SSHConnector::AuthenticateServer(serverHashCallbackType unknownCallback,
                                      serverHashCallbackType otherCallback,
                                      serverHashCallbackType changedCallback,
                                      serverHashCallbackType errorCallback)
{
    auto key = ssh_key_new();
    unsigned char* hash = NULL;
    size_t len;

    if (ssh_get_server_publickey(session, &key) != SSH_OK)
        return false;
    if (ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_SHA1, &hash, &len) != 0)
    {
        ssh_key_free(key);
        return false;
    }
    ssh_key_free(key);
    char* hex = ssh_get_fingerprint_hash(SSH_PUBLICKEY_HASH_SHA1, hash, len);
    std::string keyHash = hex;
    ssh_clean_pubkey_hash(&hash);
    ssh_string_free_char(hex);

    switch(ssh_session_is_known_server(session))
    {
    case SSH_KNOWN_HOSTS_OK:
        return true;
    case SSH_KNOWN_HOSTS_NOT_FOUND:
    case SSH_KNOWN_HOSTS_UNKNOWN:
        if (unknownCallback(keyHash))
        {
            if(ssh_session_update_known_hosts(session) == SSH_ERROR)
            {
                errorCallback(keyHash);
                return false;
            }
            return true;
        }
        else
        {
            return false;
        }
        break;
    case SSH_KNOWN_HOSTS_OTHER:
        return otherCallback(keyHash);
    case SSH_KNOWN_HOSTS_CHANGED:
        return changedCallback(keyHash);
    case SSH_KNOWN_HOSTS_ERROR:
        errorCallback(keyHash);
        return false;
    default:
        break;
    }
    return false;
}

bool SSHConnector::AuthenticateUser(passProviderType passProvider,
                                    interactiveProviderType interactiveProvider,
                                    keyProviderType keyProvider,
                                    std::string& passPrompt, std::string& keyPrompt)
{
    if (retryCount >= 3)
    {
        authStatus = AUTH_STATUS_ERROR;
        return false;
    }

    bool isCanceled = false;
    switch(authStatus)
    {
    case AUTH_STATUS_NONE:
        return AuthenticateUserNone();
    case AUTH_STATUS_CHALLENGE:
    case AUTH_STATUS_PARTIAL:
        if (authMethods == 0)
        {
            authStatus = AUTH_STATUS_ERROR;
            return false;
        }
        if (authMethods & SSH_AUTH_METHOD_PUBLICKEY)
        {
            ssh_key key = ssh_key_new();
            bool autoAuthed = AuthenticateUserKeyAuto();
            if (autoAuthed || authStatus == AUTH_STATUS_ERROR)
                return autoAuthed;
            if(!AuthenticateUserKeyFetch(keyProvider, keyPrompt, &key))
                return false;
            bool res = AuthenticateUserKey(&key);
            ssh_key_free(key);
            return res;
        }
        if (authMethods & SSH_AUTH_METHOD_PASSWORD)
        {
            std::string password = passProvider(isCanceled, passPrompt);
            if (isCanceled)
            {
                authStatus = AUTH_STATUS_ERROR;
                return false;
            }
            return AuthenticateUserPass(password);
        }
        if (authMethods & SSH_AUTH_METHOD_INTERACTIVE)
        {
            std::string name, instruction;
            int nprompts;
            bool fetchResult = AuthenticateUserInteractiveFetch(name, instruction, nprompts);
            if (!fetchResult || authStatus != AUTH_STATUS_CHALLENGE)
                return fetchResult;
            return AuthenticateUserInteractive(interactiveProvider, name, instruction, nprompts);
        }
        if (authMethods & SSH_AUTH_METHOD_GSSAPI_MIC)
        {
//#ifdef WITH_GSSAPI
            return AuthenticateGSSAPI();
//#else
//            authStatus = AUTH_STATUS_UNHANDLED;
//            return false;
//#endif
        }
        if (authMethods & SSH_AUTH_METHOD_HOSTBASED)
        {
            authStatus = AUTH_STATUS_UNHANDLED;
            return false;
        }
        break;
    case AUTH_STATUS_OK:
        return true;
    default:
        break;
    }

    return false;
}

bool SSHConnector::AuthenticateResult(int rc)
{
    switch(rc)
    {
    case SSH_AUTH_ERROR:
        authStatus = AUTH_STATUS_ERROR;
        return false;
    case SSH_AUTH_DENIED:
        isAuthDenied = true;
        retryCount += 1;
        return false;
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, NULL);
        authStatus = AUTH_STATUS_PARTIAL;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AUTH_STATUS_OK;
        isAuthDenied = false;
        retryCount = 0;
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
        break;
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

    authStatus = AUTH_STATUS_ERROR;
    return false;
}

bool SSHConnector::AuthenticateUserPass(std::string password)
{
    return AuthenticateResult(ssh_userauth_password(session, NULL, password.c_str()));
}

bool SSHConnector::AuthenticateUserKeyAuto()
{
    switch(ssh_userauth_agent(session, NULL))
    {
    case SSH_AUTH_ERROR:
        authStatus = AUTH_STATUS_ERROR;
        return false;
    case SSH_AUTH_DENIED:
        return false;
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, NULL);
        authStatus = AUTH_STATUS_PARTIAL;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AUTH_STATUS_OK;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    default:
        break;
    }

    return false;
}

bool SSHConnector::AuthenticateUserKeyFetch(keyProviderType provider, std::string keyPrompt, ssh_key* pPrivate)
{
    bool isCanceled = false;
    for (auto const& entry: std::filesystem::recursive_directory_iterator(
        Utils::GetHomePath() + "/.ssh/",
        std::filesystem::directory_options::follow_directory_symlink
            | std::filesystem::directory_options::skip_permission_denied))
    {
        std::string path = entry.path().string();
        size_t pos;
        if ((pos = path.rfind(".pub")) == std::string::npos)
            continue;
        std::string privPath = path.substr(0, pos);
        ssh_key key = ssh_key_new();

        if (ssh_pki_import_pubkey_file(path.c_str(), &key) != SSH_OK)
        {
            ssh_key_free(key);
            continue;
        }
        
        if (ssh_userauth_try_publickey(session, NULL, key) != SSH_AUTH_SUCCESS)
        {
            ssh_key_free(key);
            continue;
        }

        std::string passphrase;
        std::string prompt;
        switch(ssh_pki_import_privkey_file(privPath.c_str(), NULL, NULL, NULL, pPrivate))
        {
        case SSH_ERROR:
            prompt = keyPrompt + privPath + ":";
            passphrase = provider(isCanceled, prompt);
            if (isCanceled)
            {
                ssh_key_free(key);
                authStatus = AUTH_STATUS_ERROR;
                return false;
            }
            if (ssh_pki_import_privkey_file(privPath.c_str(), passphrase.c_str(), NULL, NULL, pPrivate) != SSH_OK)
            {
                isAuthDenied = true;
                retryCount += 1;
                ssh_key_free(key);
                return false;
            }
            break;
        case SSH_EOF:
            ssh_key_free(key);
            continue;
        case SSH_OK:
        default:
            break;
        }

        ssh_key_free(key);
        return true;
    }

    authStatus = AUTH_STATUS_ERROR;
    return false;
}

bool SSHConnector::AuthenticateUserKey(ssh_key* pPrivate)
{
    return AuthenticateResult(ssh_userauth_publickey(session, NULL, *pPrivate));
}

bool SSHConnector::AuthenticateUserInteractiveFetch(std::string& name, std::string& instruction, int& nprompts)
{
    switch(ssh_userauth_kbdint(session, NULL, NULL))
    {
    case SSH_AUTH_INFO:
        name = ssh_userauth_kbdint_getname(session);
        instruction = ssh_userauth_kbdint_getinstruction(session);
        nprompts = ssh_userauth_kbdint_getnprompts(session);
        authStatus = AUTH_STATUS_CHALLENGE;
        return true;
    case SSH_AUTH_ERROR:
        authStatus = AUTH_STATUS_ERROR;
        return false;
    case SSH_AUTH_DENIED:
        isAuthDenied = true;
        retryCount += 1;
        return false;
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, NULL);
        authStatus = AUTH_STATUS_PARTIAL;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AUTH_STATUS_OK;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    default:
        break;
    }
    return false;
}

bool SSHConnector::AuthenticateUserInteractive(interactiveProviderType provider, std::string& name, std::string& instruction, int nprompts)
{
    bool isCanceled;
    const char* prompt;
    char shouldNotBeHidden;
    std::string format = name.size() && instruction.size()? "{}\n{}\n{}": (name.size() || instruction.size()? "{}{}\n{}" : "{}{}{}");
    for (int i = 0; i < nprompts; i++)
    {
        prompt = ssh_userauth_kbdint_getprompt(session, i, &shouldNotBeHidden);
        std::string challenge = fmt::format(format, name, instruction, prompt);
        std::string res = provider(isCanceled, challenge, !(bool)shouldNotBeHidden);
        if (isCanceled || ssh_userauth_kbdint_setanswer(session, i, res.c_str()) < 0)
        {
            authStatus = AUTH_STATUS_ERROR;
            return false;
        }
    }
    return true;
}


bool SSHConnector::AuthenticateGSSAPI()
{
    return AuthenticateResult(ssh_userauth_gssapi(session));
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

std::vector<FileNode> SSHConnector::CallCLICreep(std::string dirToCreep)
{
    std::vector<FileNode> nodes;
    auto pChannel = CallCLI(fmt::format("-c {}", dirToCreep));

    unsigned char buf2[FileNode::MaxBinarySize];
    unsigned short len = 0;
    std::size_t nnodes = 0;
    if (ssh_channel_read(pChannel, &nnodes, sizeof(nnodes), 0) != sizeof(nnodes))
    {
        FreeChannel(pChannel);
        return nodes;
    }
    LOG(INFO) << "Receiving " << nnodes << " nodes...";
    while (nnodes > 0)
    {
        ssh_channel_read(pChannel, &len, sizeof(len), 0);
        ssh_channel_read(pChannel, &buf2, len, 0);
        auto node = FileNode::Deserialize(buf2);
        nodes.push_back(node);
        nnodes--;
    }

    FreeChannel(pChannel);
    return nodes;
}
#undef BUFFER_SIZE
