#pragma once
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <string>
#include <forward_list>

#include "Lib/SSHConnector.h"

/*A class for managing SSH File Transfer Protocol communications.*/
class SFTPConnector
{
public:
    SFTPConnector(SSHConnector* ssh);
    ~SFTPConnector();

    bool Connect();
    void EndSession();

    bool Send(std::string localPath, std::string remotePath, std::string tempPath, std::string hashedPath, off_t size);
    bool Receive(std::string localPath, std::string remotePath, std::string hashedPath, off_t size);
    bool Delete(std::string path);

private:
    sftp_session sftp = NULL;
    SSHConnector* ssh;
};
