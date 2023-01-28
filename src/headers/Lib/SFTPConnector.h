#ifndef SRC_LIB_SFTP_CONNECTOR_H
#define SRC_LIB_SFTP_CONNECTOR_H
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

    bool Send(const std::string localPath, const std::string tempFileName, const off_t size) const;
    bool Receive(const std::string remotePath, const std::string tempFileName, const off_t size) const;
    bool ReceiveNonAtomic(const std::string remotePath, const std::string localPath) const ;
    bool Delete(const std::string path) const;
    sftp_attributes Stat(const std::string path) const;
    bool IsAbsent() const;

private:
    sftp_session sftp = nullptr;
    SSHConnector* ssh;
};

#endif
