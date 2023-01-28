#include "Lib/SFTPConnector.h"

#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include "Lib/FileSystem.h"
#include "Utils.h"

SFTPConnector::SFTPConnector(SSHConnector* ssh)
{
    this->ssh = ssh;
}

SFTPConnector::~SFTPConnector()
{
    EndSession();
}

bool SFTPConnector::Connect()
{
    if (!ssh)
        return false;

    if ((sftp = ssh->MakeSFTPSession()) == nullptr)
        return false;
    
    if (sftp_init(sftp) != SSH_OK)
    {
        sftp_free(sftp);
        return false;
    }
    
    return true;
}

void SFTPConnector::EndSession()
{
    if (sftp != nullptr)
    {
        sftp_free(sftp);
        sftp = nullptr;
    }
}

static const int bufferSize = 4096 * 4;

bool SFTPConnector::Send(const std::string localPath, const std::string tempFileName, const off_t size) const
{
    int len = 0;
    int len2 = 0;
    off_t bytesleft = size;
    std::vector<unsigned char> buf(bufferSize, 0);

    // open both
    sftp_file pFile = sftp_open(sftp, tempFileName.c_str(), O_CREAT | O_WRONLY, S_IRWXU|S_IRGRP|S_IROTH);
    if (pFile == nullptr)
    {
        LOG(ERROR) << "Failed to open remote file " << tempFileName << " with error: " << (ssh? ssh->GetError(): "");
        return false;
    }
    int localFd = open(localPath.c_str(), O_RDONLY);
    if (localFd == -1)
    {
        int err = errno;
        LOG(ERROR) << "Error opening local file " << localPath << ". Message: " << strerror(err);
        sftp_close(pFile);
        return false;
    }

    // check if we're resuming a failed operation
    int initlen = sftp_stat(sftp, tempFileName.c_str())->size;
    if (initlen > 0)
    {
        bytesleft -= initlen;
        lseek(localFd, initlen, SEEK_SET);
        sftp_seek(pFile, initlen);
    }

    // write
    while (bytesleft > 0)
    {
        if (len != len2)
            lseek(localFd, len2 - len, SEEK_CUR);

        len = read(localFd, buf.data(), buf.size());
        if (len < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error reading local file " << localPath << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }

        len2 = sftp_write(pFile, buf.data(), len);
        if (len2 < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error writing remote file " << tempFileName << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }
        bytesleft -= len2;
    }

    if (sftp_close(pFile) == SSH_ERROR)
    {
        LOG(ERROR) << "Failed to close remote file because: " << (ssh? ssh->GetError(): "");
        return false;
    }
    close(localFd);

    return true;
}

bool SFTPConnector::Receive(const std::string remotePath, const std::string tempFileName, const off_t size) const
{
    int len = 0;
    int len2 = 0;
    off_t bytesleft = size;
    std::vector<unsigned char> buf(bufferSize, 0);

    // open both
    sftp_file pFile = sftp_open(sftp, remotePath.c_str(), O_RDONLY, S_IRWXU);
    if (pFile == nullptr)
    {
        LOG(ERROR) << "Failed to open remote file " << remotePath << " with error: " << (ssh? ssh->GetError(): "");
        return false;
    }
    int localFd = open(tempFileName.c_str(), O_WRONLY | O_CREAT, S_IRWXU|S_IRGRP|S_IROTH);
    if (localFd == -1)
    {
        int err = errno;
        LOG(ERROR) << "Failed to open temp file " << tempFileName << " with error: " << strerror(err);
        sftp_close(pFile);
        return false;
    }

    // check if we're resuming a failed operation
    struct stat buf2;
    fstat(localFd, &buf2);
    int initlen = buf2.st_size;
    if (initlen > 0)
    {
        bytesleft -= initlen;
        sftp_seek(pFile, initlen);
        lseek(localFd, initlen, SEEK_SET);
    }

    // write
    while (bytesleft > 0)
    {
        if (len != len2)
            sftp_seek(pFile, len2 - len);

        len = sftp_read(pFile, buf.data(), buf.size());
        if (len < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error reading remote file " << remotePath << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;            
        }
        len2 = write(localFd, buf.data(), len);
        if (len2 < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error writing temp file " << tempFileName << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }
        bytesleft -= len2;
    }

    sftp_close(pFile);
    close(localFd);
    return true;
}

bool SFTPConnector::ReceiveNonAtomic(const std::string remotePath, const std::string localPath) const
{
    struct stat buf2;
    if (stat(localPath.c_str(), &buf2) == 0)
        return true;

    int len = 0;
    int len2 = 0;
    std::vector<unsigned char> buf(bufferSize, 0);

    // open both
    sftp_file pFile = sftp_open(sftp, remotePath.c_str(), O_RDONLY, S_IRWXU);
    if (pFile == nullptr)
    {
        LOG(ERROR) << "Failed to open remote file " << remotePath << " with error: " << (ssh? ssh->GetError(): "");
        return false;
    }
    int localFd = open(localPath.c_str(), O_WRONLY | O_CREAT, S_IRWXU|S_IRGRP|S_IROTH);
    if (localFd == -1)
    {
        int err = errno;
        LOG(ERROR) << "Failed to open local file " << localPath << " with error: " << strerror(err);
        sftp_close(pFile);
        return false;
    }

    // check if we're resuming a failed operation
    off_t size = sftp_stat(sftp, remotePath.c_str())->size;
    int initlen = buf2.st_size;
    if (initlen > 0)
    {
        size -= initlen;
        sftp_seek(pFile, initlen);
        lseek(localFd, initlen, SEEK_SET);
    }

    // write
    while (size > 0)
    {
        if (len != len2)
            sftp_seek(pFile, len2 - len);

        len = sftp_read(pFile, buf.data(), buf.size());
        if (len < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error reading remote file " << remotePath << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }
        len2 = write(localFd, buf.data(), len);
        if (len2 < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error writing local file " << localPath << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }
        size -= len2;
    }

    sftp_close(pFile);
    close(localFd);

    return true;
}

bool SFTPConnector::Delete(const std::string path) const
{
    return sftp_unlink(sftp, path.c_str()) == SSH_OK;
}

sftp_attributes SFTPConnector::Stat(const std::string path) const
{
    return sftp_stat(sftp, path.c_str());
}

bool SFTPConnector::IsAbsent() const
{
    return sftp_get_error(sftp) == ENOENT;
}
