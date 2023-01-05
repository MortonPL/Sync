#include "Lib/SFTPConnector.h"

#include <fcntl.h>
#include <filesystem>
#include <iostream>
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

bool SFTPConnector::Send(std::string localPath, std::string remotePath, std::string tempPath, std::string hashedPath, off_t size)
{
    sftp_file pFile;
    int localFd;
    int len = 0;
    int len2 = 0;
    int initlen = 0;
    char buf[BUFSIZ];
    std::string fullTempPath = tempPath + hashedPath + ".SyncTEMP";
    // open both
    if ((pFile = sftp_open(sftp, fullTempPath.c_str(), O_CREAT | O_WRONLY, S_IRWXU|S_IRGRP|S_IROTH)) == NULL)
    {
        LOG(ERROR) << "Failed to open remote file " << fullTempPath << " with error: " << ssh->GetError();
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
    if ((initlen = sftp_stat(sftp, fullTempPath.c_str())->size) > 0)
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
            LOG(ERROR) << "Error writing remote file " << remotePath << ". Message: " << strerror(err);
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

bool SFTPConnector::Receive(std::string localPath, std::string remotePath, std::string hashedPath, off_t size)
{
    sftp_file pFile;
    int localFd;
    int len = 0;
    int len2 = 0;
    int initlen = 0;
    char buf[BUFSIZ];
    off_t size2 = size;
    std::string fullTempPath = Utils::GetTempPath() + hashedPath + ".SyncTEMP";
    // open both
    if ((pFile = sftp_open(sftp, remotePath.c_str(), O_RDONLY, S_IRWXU)) == NULL)
    {
        LOG(ERROR) << "Failed to open remote file " << remotePath << " with error: " << ssh->GetError();
        return false;
    }
    if ((localFd = open(fullTempPath.c_str(), O_WRONLY | O_CREAT, S_IRWXU|S_IRGRP|S_IROTH)) == -1)
    {
        int err = errno;
        LOG(ERROR) << "Failed to open temp file " << fullTempPath << " with error: " << strerror(err);
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
            LOG(ERROR) << "Error writing local file " << localPath << ". Message: " << strerror(err);
            sftp_close(pFile);
            close(localFd);
            return false;
        }
        size -= len2;
    }

    sftp_close(pFile);
    close(localFd);

    // move
    auto path = std::filesystem::path(localPath);
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path());
    // try to move atomically, if it fails, copy the old fashioned way
    if (rename(fullTempPath.c_str(), localPath.c_str()) < 0)
    {
        // error!
        int err = errno;
        LOG(ERROR) << "Error moving temporary file " << fullTempPath << ". Message: " << strerror(err);
        if (err == 18)
        {
            if ((localFd = open(fullTempPath.c_str(), O_RDONLY)) == -1);
            {
                int err = errno;
                LOG(ERROR) << "Failed to open temp file " << fullTempPath << " with error: " << strerror(err);
                sftp_close(pFile);
                return false;
            }
            int destFd;
            if ((destFd = open(localPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH)) == -1)
            {
                int err = errno;
                LOG(ERROR) << "Failed to open destination file " << fullTempPath << " with error: " << strerror(err);
                sftp_close(pFile);
                return false;
            }

            while (size2 > 0)
            {
                if ((len = read(localFd, buf, sizeof(buf))) < 0)
                {
                    // error!
                    int err = errno;
                    LOG(ERROR) << "Error reading temp file " << fullTempPath << ". Message: " << strerror(err);
                    close(destFd);
                    close(localFd);
                    return false;            
                }
                if ((len2 = write(destFd, buf, len)) < 0)
                {
                    // error!
                    int err = errno;
                    LOG(ERROR) << "Error writing local file " << localPath << ". Message: " << strerror(err);
                    close(destFd);
                    close(localFd);
                    return false;
                }
                if (len != len2)
                {
                    // error!
                    int err = errno;
                    LOG(ERROR) << "Error moving local file " << localPath << ". Read more bytes than written!";
                    close(destFd);
                    close(localFd);
                    return false;
                }
                size2 -= len2;
            }

            remove(fullTempPath.c_str());
            return true;
        }
        return false;
    }

    return true;
}
#undef BUFFER_SIZE

bool SFTPConnector::Delete(std::string path)
{
    return sftp_unlink(sftp, path.c_str()) == SSH_OK;
}
