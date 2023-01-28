#include "Lib/SSHConnector.h"

#include <limits.h>
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
    if (session != nullptr)
    {
        if (!(ssh_is_connected(session) && (authStatus == AuthStatus::Ok)))
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

    const std::string passPrompt = fmt::format("Enter password for {}@{}:", user, address);
    const std::string keyPrompt = "Enter passphrase for private key ";

    while(authStatus != AuthStatus::Ok)
    {
        while (!AuthenticateUser(passwordProvider, interactiveProvider, keyProvider, passPrompt, keyPrompt)
               && (authStatus != AuthStatus::Error && authStatus != AuthStatus::Unhandled))
        {
            if (isAuthDenied)
                genericMessenger("Permission denied. Please try again.");
        }

        if (authStatus == AuthStatus::Error)
        {
            genericMessenger("Failed to connect.");
            EndSession();
            return false;
        }
        else if (authStatus == AuthStatus::Unhandled)
        {
            genericMessenger("Server requires an authentication method that is not supported (hostbased or gssapi).");
            EndSession();
            return false;
        }
    }

    return true;
}

bool SSHConnector::BeginSession(const std::string host, const std::string user)
{
    session = ssh_new();
    if (session == nullptr)
        return false;
    long timeout = 10*60;
    ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, user.c_str());
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout);
    if (ssh_connect(session) != SSH_OK)
    {
        ssh_free(session);
        session = nullptr;
        return false;
    }
    return true;
}

void SSHConnector::EndSession()
{
    authStatus = AuthStatus::None;
    authMethods = 0;
    isAuthDenied = false;
    retryCount = 0;
    if (channel != nullptr)
    {
        if (ssh_channel_is_open(channel))
            EndCLIServe();
        else
            channel = nullptr;
    }
    if (session != nullptr)
    {
        if (ssh_is_connected(session))
            ssh_disconnect(session);
        ssh_free(session);
        session = nullptr;
    }
}

sftp_session SSHConnector::MakeSFTPSession()
{
    return sftp_new(session);
}

int SSHConnector::CallCLICreep(std::string dirToCreep)
{
    if ((channel = CallCLI("c", dirToCreep)) == nullptr)
    {
        FreeChannel(channel);
        return CALLCLI_NOANSWER;
    }

    char rc;
    // read return code for creep
    if (ssh_channel_read(channel, &rc, sizeof(rc), 0) != sizeof(rc))
    {
        FreeChannel(channel);
        return CALLCLI_404;
    }
    if (rc != (char)Messages::Ok)
    {
        FreeChannel(channel);
        return CALLCLI_ERROR;
    }

    return CALLCLI_OK;
}

int SSHConnector::CallCLICreepReturn(std::forward_list<FileNode>& nodes)
{
    FileNode::MarshallingContainer buf;
    // prealocate 4096 to avoid resizing frequently
    buf.reserve(PATH_MAX);
    std::size_t len = 0;
    char rc;
    std::size_t nnodes = 0;
    // read return code for creep
    if (ssh_channel_read(channel, &rc, sizeof(rc), 0) != sizeof(rc))
    {
        FreeChannel(channel);
        return CALLCLI_404;
    }
    if (rc != (char)Messages::Ok)
    {
        FreeChannel(channel);
        return CALLCLI_ERROR;
    }
    // ack
    if (ssh_channel_write(channel, &rc, sizeof(rc)) != sizeof(rc))
    {
        FreeChannel(channel);
        return CALLCLI_ERROR;
    }
    // read node count
    if (ssh_channel_read(channel, &nnodes, sizeof(nnodes), 0) != sizeof(nnodes))
    {
        FreeChannel(channel);
        return CALLCLI_ERROR;
    }
    // read nodes
    LOG(INFO) << "Receiving " << nnodes << " nodes...";
    std::size_t i = 0;
    while (nnodes > 0)
    {
        if (i >= 15000)
            ssh_channel_write(channel, &rc, sizeof(rc));

        ssh_channel_read(channel, &len, sizeof(len), 0);
        if (buf.size() < len)
            buf.resize(len);
        ssh_channel_read(channel, buf.data(), len, 0);
        auto node = FileNode::Deserialize(buf);
        nodes.push_front(node);
        nnodes--;
        i++;
    }

    FreeChannel(channel);
    return CALLCLI_OK;
}

int SSHConnector::CallCLIHomeAndBlock(std::string pathToCheck, std::string& result)
{
    ssh_channel pChannel;
    if ((pChannel = CallCLI("h", pathToCheck)) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    unsigned short len;
    if (ssh_channel_read(pChannel, &len, sizeof(len), 0) != sizeof(len))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }

    std::string buf(len, 0);
    ssh_channel_read(pChannel, buf.data(), len, 0);
    result = buf;

    bool blocked;
    ssh_channel_read(pChannel, &blocked, sizeof(blocked), 0);

    FreeChannel(pChannel);
    return blocked? CALLCLI_OK: CALLCLI_BLOCKED;
}

int SSHConnector::CallCLIHome(std::string& result)
{
    ssh_channel pChannel;
    if ((pChannel = CallCLI("h")) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    unsigned short len;
    if (ssh_channel_read(pChannel, &len, sizeof(len), 0) != sizeof(len))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }

    std::string buf(len, 0);
    ssh_channel_read(pChannel, buf.data(), len, 0);
    result = buf;

    FreeChannel(pChannel);
    return CALLCLI_OK;
}

int SSHConnector::CallCLIUnblock(std::string path)
{
    ssh_channel pChannel;
    if ((pChannel = CallCLI("u", path)) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    bool unblocked;
    if (ssh_channel_read(pChannel, &unblocked, sizeof(unblocked), 0) != sizeof(unblocked))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }

    FreeChannel(pChannel);
    return unblocked? CALLCLI_OK: CALLCLI_ERROR;
}

int SSHConnector::CallCLICompress(std::string pathFrom, std::string pathTo, off_t* compressedSize) const
{
    ssh_channel pChannel;
    if ((pChannel = CallCLI("z", pathFrom, pathTo)) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    char compressed;
    if (ssh_channel_read(pChannel, &compressed, sizeof(compressed), 0) != sizeof(compressed))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }
    if (compressed != (char)Messages::Ok)
    {
        FreeChannel(pChannel);
        return CALLCLI_ERROR;
    }

    if (ssh_channel_read(pChannel, compressedSize, sizeof(*compressedSize), 0) != sizeof(*compressedSize))
    {
        FreeChannel(pChannel);
        return CALLCLI_ERROR;
    }

    FreeChannel(pChannel);
    return (*compressedSize != 0? CALLCLI_OK: CALLCLI_ERROR);
}

int SSHConnector::CallCLIDecompress(std::string pathFrom, std::string pathTo)
{
    ssh_channel pChannel;
    if ((pChannel = CallCLI("Z", pathFrom, pathTo)) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    char decompressed;
    if (ssh_channel_read(pChannel, &decompressed, sizeof(decompressed), 0) != sizeof(decompressed))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }
    if (decompressed != (char)Messages::Ok)
    {
        FreeChannel(pChannel);
        return CALLCLI_ERROR;
    }

    if (ssh_channel_read(pChannel, &decompressed, sizeof(decompressed), 0) != sizeof(decompressed))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }

    FreeChannel(pChannel);
    return (decompressed == (char)Messages::Ok? CALLCLI_OK: CALLCLI_ERROR);
}

// NOTE - unused and unfinished
int SSHConnector::CallCLIServe()
{
    FreeChannel(channel);
    if ((channel = CallCLI("d", "")) == nullptr)
    {
        return CALLCLI_NOANSWER;
    }
    char rc;
    if (ssh_channel_read(channel, &rc, sizeof(rc), 0) != sizeof(rc) || rc != 'd')
    {
        FreeChannel(channel);
        return CALLCLI_404;
    }
    return CALLCLI_OK;
}

// NOTE - unused and unfinished
int SSHConnector::EndCLIServe()
{
    char buf = 0;
    ssh_channel_write(channel, &buf, sizeof(buf));
    FreeChannel(channel);

    return CALLCLI_OK;
}

// NOTE - unused and unfinished
int SSHConnector::ServerStatRemote(std::string pathToStat, struct stat* pStatBuf)
{
    pathToStat.size();

    if (!ssh_channel_is_open(channel))
    {
        if (CallCLIServe() != CALLCLI_OK)
            return CALLCLI_ERROR;
    }

    char rc = 's';
    if (ssh_channel_write(channel, &rc, sizeof(rc)) != sizeof(rc))
    {
        EndCLIServe();
        return CALLCLI_ERROR;
    }

    FileNode::MarshallingContainer buf(FileNode::MiniStatBinarySize, FileNode::MarshallingUnit(0));
    
    if (ssh_channel_read(channel, &rc, sizeof(rc), 0) != sizeof(rc))
    {
        EndCLIServe();
        return CALLCLI_404;
    }
    if (rc != '0')
    {
        EndCLIServe();
        return CALLCLI_ERROR;
    }
    if ((std::size_t)ssh_channel_read(channel, buf.data(), FileNode::MiniStatBinarySize, 0) != FileNode::MiniStatBinarySize)
    {
        EndCLIServe();
        return CALLCLI_404;
    }
    FileNode::DeserializeStat(buf, pStatBuf);
    return CALLCLI_OK;
}

int SSHConnector::StatRemote(std::string pathToStat, struct stat* pStatBuf)
{
    ssh_channel_struct* pChannel;
    if ((pChannel = CallCLI("s", pathToStat)) == nullptr)
    {
        FreeChannel(pChannel);
        return CALLCLI_NOANSWER;
    }
    FileNode::MarshallingContainer buf(FileNode::MiniStatBinarySize, FileNode::MarshallingUnit(0));
    char rc;
    if (ssh_channel_read(pChannel, &rc, sizeof(rc), 0) != sizeof(rc))
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }
    if (rc != (char)Messages::Stat)
    {
        FreeChannel(pChannel);
        return CALLCLI_ERROR;
    }
    if ((std::size_t)ssh_channel_read(pChannel, buf.data(), FileNode::MiniStatBinarySize, 0) != FileNode::MiniStatBinarySize)
    {
        FreeChannel(pChannel);
        return CALLCLI_404;
    }
    FileNode::DeserializeStat(buf, pStatBuf);
    FreeChannel(pChannel);
    return CALLCLI_OK;
}

int SSHConnector::ReplaceFile(std::string pathFrom, std::string pathTo)
{
    auto pChannel = GetChannel();
    auto path = std::filesystem::path(pathTo).parent_path().string();
    char buf;
    Utils::Replace(path, "\'", "\'\\\'\'");

    if (ssh_channel_request_exec(pChannel, ("mkdir -p \'" + path + "\'; echo 0;").c_str()) != SSH_OK)
    {
        LOG(ERROR) << ssh_get_error(session);
        FreeChannel(pChannel);
        return CALLCLI_NOANSWER;
    }
    if ((ssh_channel_read(pChannel, &buf, sizeof(buf), 0) != sizeof(buf)) || buf != '0')
    {
        LOG(ERROR) << ssh_get_error(session);
        FreeChannel(pChannel);
        return CALLCLI_ERROR;
    }
    FreeChannel(pChannel);

    pChannel = GetChannel();
    if (ssh_channel_request_exec(pChannel, ("mv " + pathFrom + " \'" + pathTo + "\'; echo 0;").c_str()) != SSH_OK)
    {
        LOG(ERROR) << ssh_get_error(session);
        FreeChannel(pChannel);
        return CALLCLI_NOANSWER;
    }
    if ((ssh_channel_read(pChannel, &buf, sizeof(buf), 0) != sizeof(buf)) || buf != '0')
    {
        LOG(ERROR) << ssh_get_error(session);
        FreeChannel(pChannel);
        return CALLCLI_ERROR;
    }

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
    unsigned char* hash = nullptr;
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
                                    const std::string& passPrompt, const std::string& keyPrompt)
{
    if (retryCount >= 3)
    {
        authStatus = AuthStatus::Error;
        return false;
    }

    switch(authStatus)
    {
    case AuthStatus::None:
        return AuthenticateUserNone();
    case AuthStatus::Challenge:
    case AuthStatus::NoKey:
    case AuthStatus::Partial:
        if (authMethods == 0)
        {
            authStatus = AuthStatus::Error;
            return false;
        }
        if ((authMethods & SSH_AUTH_METHOD_PUBLICKEY) && authStatus != AuthStatus::NoKey)
        {
            ssh_key key = ssh_key_new();
            bool autoAuthed = AuthenticateUserKeyAuto();
            if (autoAuthed || authStatus == AuthStatus::Error)
                return autoAuthed;
            if(!AuthenticateUserKeyFetch(keyProvider, keyPrompt, &key))
                return false;
            bool res = AuthenticateUserKey(&key);
            ssh_key_free(key);
            return res;
        }
        if (authMethods & SSH_AUTH_METHOD_PASSWORD)
        {
            bool isCanceled = false;
            const std::string password = passProvider(isCanceled, passPrompt);
            if (isCanceled)
            {
                authStatus = AuthStatus::Error;
                return false;
            }
            return AuthenticateUserPass(password);
        }
        if (authMethods & SSH_AUTH_METHOD_INTERACTIVE)
        {
            std::string name, instruction;
            int nprompts;
            bool fetchResult = AuthenticateUserInteractiveFetch(name, instruction, nprompts);
            if (!fetchResult || authStatus != AuthStatus::Challenge)
                return fetchResult;
            return AuthenticateUserInteractive(interactiveProvider, name, instruction, nprompts);
        }
        if (authMethods & SSH_AUTH_METHOD_GSSAPI_MIC)
        {
            authStatus = AuthStatus::Unhandled;
            return false;
        }
        if (authMethods & SSH_AUTH_METHOD_HOSTBASED)
        {
            authStatus = AuthStatus::Unhandled;
            return false;
        }
        break;
    case AuthStatus::Ok:
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
        authStatus = AuthStatus::Error;
        return false;
    case SSH_AUTH_DENIED:
        isAuthDenied = true;
        retryCount += 1;
        return false;
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, nullptr);
        authStatus = AuthStatus::Partial;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AuthStatus::Ok;
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
    switch(ssh_userauth_none(session, nullptr))
    {
    case SSH_AUTH_ERROR:
        break;
    case SSH_AUTH_DENIED:
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, nullptr);
        authStatus = AuthStatus::Partial;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AuthStatus::Ok;
        return true;
    default:
        break;
    }

    authStatus = AuthStatus::Error;
    return false;
}

bool SSHConnector::AuthenticateUserPass(const std::string password)
{
    return AuthenticateResult(ssh_userauth_password(session, nullptr, password.c_str()));
}

bool SSHConnector::AuthenticateUserKeyAuto()
{
    switch(ssh_userauth_agent(session, nullptr))
    {
    case SSH_AUTH_ERROR:
        authStatus = AuthStatus::Error;
        return false;
    case SSH_AUTH_DENIED:
        return false;
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, nullptr);
        authStatus = AuthStatus::Partial;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AuthStatus::Ok;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    default:
        break;
    }

    return false;
}

bool SSHConnector::AuthenticateUserKeyFetch(keyProviderType provider, const std::string keyPrompt, ssh_key* pPrivate)
{
    bool isCanceled = false;
    for (auto const& entry: std::filesystem::recursive_directory_iterator(
        Utils::GetHomePath() + "/.ssh/",
        std::filesystem::directory_options::follow_directory_symlink
            | std::filesystem::directory_options::skip_permission_denied))
    {
        const std::string path = entry.path().string();
        size_t pos = path.rfind(".pub");
        if (pos == std::string::npos)
            continue;
        const std::string privPath = path.substr(0, pos);
        ssh_key key = ssh_key_new();

        if (ssh_pki_import_pubkey_file(path.c_str(), &key) != SSH_OK)
        {
            ssh_key_free(key);
            continue;
        }
        
        if (ssh_userauth_try_publickey(session, nullptr, key) != SSH_AUTH_SUCCESS)
        {
            ssh_key_free(key);
            continue;
        }

        std::string passphrase;
        std::string prompt;
        switch(ssh_pki_import_privkey_file(privPath.c_str(), nullptr, nullptr, nullptr, pPrivate))
        {
        case SSH_ERROR:
            prompt = keyPrompt + privPath + ":";
            passphrase = provider(isCanceled, prompt);
            if (isCanceled)
            {
                ssh_key_free(key);
                authStatus = AuthStatus::Error;
                return false;
            }
            if (ssh_pki_import_privkey_file(privPath.c_str(), passphrase.c_str(), nullptr, nullptr, pPrivate) != SSH_OK)
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

    authStatus = AuthStatus::NoKey;
    return false;
}

bool SSHConnector::AuthenticateUserKey(ssh_key* pPrivate)
{
    return AuthenticateResult(ssh_userauth_publickey(session, nullptr, *pPrivate));
}

bool SSHConnector::AuthenticateUserInteractiveFetch(std::string& name, std::string& instruction, int& nprompts)
{
    switch(ssh_userauth_kbdint(session, nullptr, nullptr))
    {
    case SSH_AUTH_INFO:
        name = ssh_userauth_kbdint_getname(session);
        instruction = ssh_userauth_kbdint_getinstruction(session);
        nprompts = ssh_userauth_kbdint_getnprompts(session);
        authStatus = AuthStatus::Challenge;
        return true;
    case SSH_AUTH_ERROR:
        authStatus = AuthStatus::Error;
        return false;
    case SSH_AUTH_DENIED:
        isAuthDenied = true;
        retryCount += 1;
        return false;
    case SSH_AUTH_PARTIAL:
        authMethods = ssh_userauth_list(session, nullptr);
        authStatus = AuthStatus::Partial;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    case SSH_AUTH_SUCCESS:
        authStatus = AuthStatus::Ok;
        isAuthDenied = false;
        retryCount = 0;
        return true;
    default:
        break;
    }
    return false;
}

bool SSHConnector::AuthenticateUserInteractive(interactiveProviderType provider, const std::string& name, const std::string& instruction, int nprompts)
{
    bool isCanceled;
    char shouldNotBeHidden;
    const std::string format = name.size() && instruction.size()? "{}\n{}\n{}": (name.size() || instruction.size()? "{}{}\n{}" : "{}{}{}");
    for (int i = 0; i < nprompts; i++)
    {
        const char* prompt = ssh_userauth_kbdint_getprompt(session, i, &shouldNotBeHidden);
        const std::string challenge = fmt::format(format, name, instruction, prompt);
        const std::string res = provider(isCanceled, challenge, !(bool)shouldNotBeHidden);
        if (isCanceled || ssh_userauth_kbdint_setanswer(session, i, res.c_str()) < 0)
        {
            authStatus = AuthStatus::Error;
            return false;
        }
    }
    return true;
}

bool SSHConnector::AuthenticateGSSAPI()
{
    return AuthenticateResult(ssh_userauth_gssapi(session));
}

ssh_channel SSHConnector::GetChannel() const
{
    auto pChannel = ssh_channel_new(session);
    if (pChannel == nullptr)
        return nullptr;
    if (ssh_channel_open_session(pChannel) != SSH_OK)
        return nullptr;
    return pChannel;
}

void SSHConnector::FreeChannel(ssh_channel pChannel) const
{
    ssh_channel_send_eof(pChannel);
    ssh_channel_close(pChannel);
    ssh_channel_free(pChannel);
}

ssh_channel SSHConnector::CallCLI(std::string flag)
{
    auto pChannel = GetChannel();
    if (ssh_channel_request_exec(pChannel, ("synccli -" + flag).c_str()) != SSH_OK)
    {
        FreeChannel(pChannel);
        return nullptr;
    }
    return pChannel;
}

ssh_channel SSHConnector::CallCLI(std::string flag, std::string cmd)
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

ssh_channel SSHConnector::CallCLI(std::string flag, std::string cmd, std::string cmd2) const
{
    auto pChannel = GetChannel();
    Utils::Replace(cmd, "\'", "\'\\\'\'");
    Utils::Replace(cmd2, "\'", "\'\\\'\'");
    if (ssh_channel_request_exec(pChannel, ("synccli -" + flag + " \'" + cmd + "\' \'" + cmd2 + "\'").c_str()) != SSH_OK)
    {
        FreeChannel(pChannel);
        return nullptr;
    }
    return pChannel;
}
