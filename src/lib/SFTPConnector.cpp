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

    if ((sftp = ssh->MakeSFTPSession()) == NULL)
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
    if (sftp != NULL)
    {
        sftp_free(sftp);
        sftp = NULL;
    }
}

static const int bufferSize = 4096 * 4;

bool SFTPConnector::Send(std::string localPath, std::string tempFileName, off_t size) const
{
    sftp_file pFile;
    int localFd;
    int len = 0;
    int len2 = 0;
    int initlen = 0;
    char buf[bufferSize];
    // open both
    if ((pFile = sftp_open(sftp, tempFileName.c_str(), O_CREAT | O_WRONLY, S_IRWXU|S_IRGRP|S_IROTH)) == NULL)
    {
        LOG(ERROR) << "Failed to open remote file " << tempFileName << " with error: " << (ssh? ssh->GetError(): "");
        return false;
    }
    if ((localFd = open(localPath.c_str(), O_RDONLY)) == -1)
    {
        int err = errno;
        LOG(ERROR) << "Error opening local file " << localPath << ". Message: " << strerror(err);
        sftp_close(pFile);
        return false;
    }

    // check if we're resuming a failed operation
    if ((initlen = sftp_stat(sftp, tempFileName.c_str())->size) > 0)
    {
        size -= initlen;
        lseek(localFd, initlen, SEEK_SET);
        sftp_seek(pFile, initlen);
    }

    // write
    while (size > 0)
    {
        if (len != len2)
            lseek(localFd, len2 - len, SEEK_CUR);
        if ((len = read(localFd, buf, sizeof(buf))) < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error reading local file " << localPath << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }
        if ((len2 = sftp_write(pFile, buf, len)) < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error writing remote file " << tempFileName << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }
        size -= len2;
    }

    if (sftp_close(pFile) == SSH_ERROR)
    {
        LOG(ERROR) << "Failed to close remote file because: " << (ssh? ssh->GetError(): "");
        return false;
    }
    close(localFd);

    return true;
}

bool SFTPConnector::Receive(std::string remotePath, std::string tempFileName, off_t size) const
{
    sftp_file pFile;
    int localFd;
    int len = 0;
    int len2 = 0;
    int initlen = 0;
    char buf[bufferSize];
    // open both
    if ((pFile = sftp_open(sftp, remotePath.c_str(), O_RDONLY, S_IRWXU)) == NULL)
    {
        LOG(ERROR) << "Failed to open remote file " << remotePath << " with error: " << (ssh? ssh->GetError(): "");
        return false;
    }
    if ((localFd = open(tempFileName.c_str(), O_WRONLY | O_CREAT, S_IRWXU|S_IRGRP|S_IROTH)) == -1)
    {
        int err = errno;
        LOG(ERROR) << "Failed to open temp file " << tempFileName << " with error: " << strerror(err);
        sftp_close(pFile);
        return false;
    }

    // check if we're resuming a failed operation
    struct stat buf2;
    fstat(localFd, &buf2);
    if ((initlen = buf2.st_size) > 0)
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

        if ((len = sftp_read(pFile, buf, sizeof(buf))) < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error reading remote file " << remotePath << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;            
        }
        if ((len2 = write(localFd, buf, len)) < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error writing temp file " << tempFileName << ". Message: " << strerror(err);
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

bool SFTPConnector::ReceiveNonAtomic(std::string remotePath, std::string localPath) const
{
    struct stat buf2;
    if (stat(localPath.c_str(), &buf2) == 0)
        return true;

    sftp_file pFile;
    int localFd;
    int len = 0;
    int len2 = 0;
    int initlen = 0;
    char buf[bufferSize];
    // open both
    if ((pFile = sftp_open(sftp, remotePath.c_str(), O_RDONLY, S_IRWXU)) == NULL)
    {
        LOG(ERROR) << "Failed to open remote file " << remotePath << " with error: " << (ssh? ssh->GetError(): "");
        return false;
    }
    if ((localFd = open(localPath.c_str(), O_WRONLY | O_CREAT, S_IRWXU|S_IRGRP|S_IROTH)) == -1)
    {
        int err = errno;
        LOG(ERROR) << "Failed to open local file " << localPath << " with error: " << strerror(err);
        sftp_close(pFile);
        return false;
    }

    // check if we're resuming a failed operation
    off_t size = sftp_stat(sftp, remotePath.c_str())->size;
    if ((initlen = buf2.st_size) > 0)
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

        if ((len = sftp_read(pFile, buf, sizeof(buf))) < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error reading remote file " << remotePath << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }
        if ((len2 = write(localFd, buf, len)) < 0)
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

bool SFTPConnector::Delete(std::string path) const
{
    return sftp_unlink(sftp, path.c_str()) == SSH_OK;
}

sftp_attributes SFTPConnector::Stat(std::string path) const
{
    return sftp_stat(sftp, path.c_str());
}

bool SFTPConnector::IsAbsent() const
{
    return sftp_get_error(sftp) == ENOENT;
}
