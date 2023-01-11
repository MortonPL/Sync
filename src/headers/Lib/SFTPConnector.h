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

    bool Send(std::string localPath, std::string tempFileName, off_t size) const;
    bool Receive(std::string remotePath, std::string tempFileName, off_t size) const;
    bool ReceiveNonAtomic(std::string remotePath, std::string localPath) const ;
    bool Delete(std::string path) const;
    sftp_attributes Stat(std::string path) const;
    bool IsAbsent() const;

private:
    sftp_session sftp = NULL;
    SSHConnector* ssh;
};
