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
    {
        if (!(ssh_is_connected(session) && (authStatus == AUTH_STATUS_OK)))
        {
            EndSession();
            return false;
        }
        else
        {
            return true;
        }
    }

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
    if (channel != NULL)
    {
        if (ssh_channel_is_open(channel))
            EndCLIServe();
        else
            channel = NULL;
    }
    if (session != NULL)
    {
        if (ssh_is_connected(session))
            ssh_disconnect(session);
        ssh_free(session);
        session = NULL;
    }
}

sftp_session SSHConnector::MakeSFTPSession()
{
    return sftp_new(session);
}

int SSHConnector::CallCLICreep(std::string dirToCreep, std::forward_list<FileNode>& nodes)
{
    ssh_channel_struct* pChannel;
    if ((pChannel = CallCLI("c", dirToCreep)) == nullptr)
    {
        FreeChannel(pChannel);
        return CALLCLI_NOANSWER;
    }

    unsigned char buf2[FileNode::MaxBinarySize];
    unsigned short len = 0;
    char rc;
    std::size_t nnodes = 0;
    // read return code for creep
    if (ssh_channel_read(pChannel, &rc, sizeof(rc), 0) != sizeof(rc))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }
    if (rc != 0)
    {
        FreeChannel(pChannel);
        return rc;
    }
    // read node count
    if (ssh_channel_read(pChannel, &nnodes, sizeof(nnodes), 0) != sizeof(nnodes))
    {
        FreeChannel(pChannel);
        return CALLCLI_ERROR;
    }
    // read nodes
    LOG(INFO) << "Receiving " << nnodes << " nodes...";
    while (nnodes > 0)
    {
        ssh_channel_read(pChannel, &len, sizeof(len), 0);
        ssh_channel_read(pChannel, buf2, len, 0);
        auto node = FileNode::Deserialize(buf2);
        nodes.push_front(node);
        nnodes--;
    }

    FreeChannel(pChannel);
    return CALLCLI_OK;
}

int SSHConnector::CallCLIHomeAndBlock(std::string pathToCheck, std::string* result)
{
    char buf[PATH_MAX];

    if ((channel = CallCLI("h", pathToCheck)) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    unsigned short len;
    if (ssh_channel_read(channel, &len, sizeof(len), 0) != sizeof(len))
    {
        FreeChannel(channel);
        return CALLCLI_404;
    }

    if (len > PATH_MAX)
        return CALLCLI_ERROR;

    ssh_channel_read(channel, buf, len, 0);
    *result = std::string(buf, len);

    bool blocked;
    ssh_channel_read(channel, &blocked, sizeof(blocked), 0);

    return blocked? CALLCLI_OK: CALLCLI_BLOCKED;
}

int SSHConnector::CallCLIUnblock(std::string path)
{
    if ((channel = CallCLI("u", path)) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    bool unblocked;
    if (ssh_channel_read(channel, &unblocked, sizeof(unblocked), 0) != sizeof(unblocked))
    {
        FreeChannel(channel);
        return CALLCLI_404;
    }

    return unblocked? CALLCLI_OK: CALLCLI_ERROR;
}

int SSHConnector::CallCLIServe()
{
    FreeChannel(channel);
    if ((channel = CallCLI("d", "")) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    char rc;
    if (ssh_channel_read(channel, &rc, sizeof(rc), 0) != sizeof(rc) || rc != 0)
    {
        FreeChannel(channel);
        return CALLCLI_404;
    }
    return CALLCLI_OK;
}

int SSHConnector::EndCLIServe()
{
    char buf = 0;
    ssh_channel_write(channel, &buf, sizeof(buf));
    FreeChannel(channel);

    return CALLCLI_OK;
}

int SSHConnector::StatRemote(std::string pathToStat, struct stat* pBuf)
{
    ssh_channel_struct* pChannel;
    if ((pChannel = CallCLI("s", pathToStat)) == nullptr)
    {
        FreeChannel(pChannel);
        return CALLCLI_NOANSWER;
    }
    unsigned char buf[FileNode::MiniStatBinarySize];
    char rc;
    if (ssh_channel_read(pChannel, &rc, sizeof(rc), 0) != sizeof(rc))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }
    if (rc != 0)
    {
        FreeChannel(pChannel);
        return CALLCLI_ERROR;
    }
    if (ssh_channel_read(pChannel, buf, sizeof(buf), 0) != sizeof(buf))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }
    FileNode::DeserializeStat(buf, pBuf);
    FreeChannel(pChannel);
    return CALLCLI_OK;
}

int SSHConnector::ReplaceFile(std::string pathFrom, std::string pathTo)
{
    auto pChannel = GetChannel();
    auto path = std::filesystem::path(pathTo).parent_path().string();
    Utils::Replace(path, "\'", "\'\\\'\'");

    if (ssh_channel_request_exec(pChannel, ("mkdir -p \'" + path + '\'').c_str()) != SSH_OK)
    {
        LOG(ERROR) << ssh_get_error(session);
        FreeChannel(pChannel);
        return CALLCLI_NOANSWER;
    }
    if (ssh_get_error_code(session) != SSH_NO_ERROR)
        return CALLCLI_ERROR;
    FreeChannel(pChannel);

    pChannel = GetChannel();
    if (ssh_channel_request_exec(pChannel, ("mv .sync/tmp/" + pathFrom + ".SyncTEMP \'" + pathTo + '\'').c_str()) != SSH_OK)
    {
        LOG(ERROR) << ssh_get_error(session);
        FreeChannel(pChannel);
        return CALLCLI_NOANSWER;
    }
    if (ssh_get_error_code(session) != SSH_NO_ERROR)
        return CALLCLI_ERROR;

    FreeChannel(pChannel);

    return CALLCLI_OK;
}

std::string SSHConnector::GetError()
{
    return ssh_get_error(session);
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

ssh_channel_struct* SSHConnector::CallCLI(std::string flag, std::string cmd)
{
    auto pChannel = GetChannel();
    Utils::Replace(cmd, "\'", "\'\\\'\'");
    if (ssh_channel_request_exec(pChannel, ("synccli -" + flag + " \'" + cmd + '\'').c_str()) != SSH_OK)
    {
        FreeChannel(pChannel);
        return nullptr;
    }
    return pChannel;
}
