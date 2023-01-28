#ifndef SRC_LIB_SSH_CONNECTOR_H
#define SRC_LIB_SSH_CONNECTOR_H
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <string>
#include <forward_list>

#include "Domain/FileNode.h"

#define CALLCLI_OK 0
#define CALLCLI_ERROR -1
#define CALLCLI_404 -2
#define CALLCLI_NOANSWER -3
#define CALLCLI_BLOCKED -4

typedef void (*genericMessengerType)(const std::string prompt);
typedef std::string (*passProviderType)(bool& isCanceled, const std::string& prompt);
typedef std::string (*interactiveProviderType)(bool& isCanceled, const std::string& challenge, bool shouldBeHidden);
typedef std::string (*keyProviderType)(bool& isCanceled, const std::string& prompt);
typedef bool (*serverHashCallbackType)(const std::string& pubkeyHash);

/*A class for managing SSH communications.*/
class SSHConnector
{
public:
    SSHConnector();
    ~SSHConnector();

    bool Connect(const std::string& address, const std::string& user,
                 genericMessengerType genericMessenger, serverHashCallbackType unknownCallback,
                 serverHashCallbackType otherCallback, serverHashCallbackType changedCallback,
                 serverHashCallbackType errorCallback, passProviderType passwordProvider,
                 interactiveProviderType interactiveProvider, keyProviderType keyProvider);
    void EndSession();
    sftp_session MakeSFTPSession();

    int CallCLICreep(std::string dirToCreep);
    int CallCLICreepReturn(std::forward_list<FileNode>& nodes);
    int CallCLIHomeAndBlock(std::string pathToCheck, std::string* result);
    int CallCLIHome(std::string* result);
    int CallCLIUnblock(std::string path);
    int CallCLICompress(std::string pathFrom, std::string pathTo, off_t* compressedSize) const;
    int CallCLIDecompress(std::string pathFrom, std::string pathTo);
    int CallCLIServe();
    int EndCLIServe();
    int StatRemote(std::string pathToStat, struct stat* pStatBuf);
    int ServerStatRemote(std::string pathToStat, struct stat* pStatBuf);
    int ReplaceFile(std::string pathFrom, std::string pathTo);

    std::string GetError();

private:
    enum class AuthStatus: char
    {
        Error = -1,
        None,
        Unhandled,
        Partial,
        Challenge,
        Ok,
        NoKey,
    };

    ssh_channel GetChannel() const;
    void FreeChannel(ssh_channel pChannel) const;
    ssh_channel CallCLI(std::string flag);
    ssh_channel CallCLI(std::string flag, std::string cmd);
    ssh_channel CallCLI(std::string flag, std::string cmd, std::string cmd2) const;

    bool BeginSession(const std::string host, const std::string user);
    bool AuthenticateServer(serverHashCallbackType unknownCallback,
                            serverHashCallbackType otherCallback,
                            serverHashCallbackType changedCallback,
                            serverHashCallbackType errorCallback);
    bool AuthenticateUser(passProviderType passwordProvider,
                          interactiveProviderType interactiveProvider,
                          keyProviderType keyProvider,
                          const std::string& passPrompt, const std::string& keyPrompt);
    bool AuthenticateUserNone();
    bool AuthenticateGSSAPI();
    bool AuthenticateUserKeyAuto();
    bool AuthenticateUserKeyFetch(keyProviderType provider, const std::string keyPrompt, ssh_key* pPrivate);
    bool AuthenticateUserKey(ssh_key* pPrivate);
    bool AuthenticateUserInteractiveFetch(std::string& name, std::string& instruction, int& nprompts);
    bool AuthenticateUserInteractive(interactiveProviderType provider, const std::string& name, const std::string& instruction, int nprompts);
    bool AuthenticateUserPass(const std::string password);
    bool AuthenticateResult(int rc);

    ssh_session session = nullptr;
    ssh_channel channel = nullptr;

    AuthStatus authStatus = AuthStatus::None;
    int authMethods = 0;
    bool isAuthDenied = false;
    int retryCount = 0;
};

#endif
