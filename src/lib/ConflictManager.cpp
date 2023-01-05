#include "Lib/ConflictManager.h"

#include <fcntl.h>
#include "Utils.h"

std::string _QuickHash(std::string value)
{
    return fmt::format("{:x}", XXH64(value.c_str(), value.size(), 0));
}

bool CopyLocalFile(const std::string& localPath, const std::string& tempPath)
{
    int localFd;
    int tempFd;
    int len;
    int len2;
    char buf[BUFSIZ];
    struct stat statbuf;
    off_t size;

    if ((localFd = open(localPath.c_str(), O_RDONLY)) == -1)
    {
        int err = errno;
        LOG(ERROR) << "Failed to open local file " << localPath << " with error: " << strerror(err);
        return false;
    }
    if ((tempFd = open(tempPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH)) == -1)
    {
        int err = errno;
        LOG(ERROR) << "Failed to open temp file " << tempPath << " with error: " << strerror(err);
        close(localFd);
        return false;
    }

    fstat(localFd, &statbuf);
    size = statbuf.st_size;

    while (size > 0)
    {
        if ((len = read(localFd, buf, sizeof(buf))) < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error reading local file " << localPath << ". Message: " << strerror(err);
            close(tempFd);
            close(localFd);
            return false;            
        }
        if ((len2 = write(tempFd, buf, len)) < 0)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error writing temp file " << tempPath << ". Message: " << strerror(err);
            close(tempFd);
            close(localFd);
            return false;
        }
        if (len != len2)
        {
            // error!
            int err = errno;
            LOG(ERROR) << "Error moving local file " << localPath << ". Read more bytes than written!";
            close(tempFd);
            close(localFd);
            return false;
        }
        size -= len2;
    }

    return true;
}

bool CopyRemoteFile(const std::string& remotePath, const std::string& tempPath, SFTPConnector& sftp)
{
    return false;
}

void ConflictManager::Resolve(PairedNode* pNode, ConflictRule& rule, SFTPConnector& sftp)
{
    if (rule.command == "")
        return;
    
    if (pNode->defaultAction != PairedNode::Action::Conflict)
        return;
    
    if (pNode->historyNode.status == FileNode::Status::DeletedLocal || pNode->historyNode.status == FileNode::Status::DeletedRemote)
        return;
    
    // get both files to tmp/
    std::string hashedPath = Utils::GetTempPath() + _QuickHash(pNode->path);
    std::string tempPathLocal = hashedPath + ".SyncLOCAL";
    std::string tempPathRemote = hashedPath + ".SyncREMOTE";
    if (!CopyLocalFile(pNode->path, tempPathLocal))
        return;
    
    if (!CopyRemoteFile(pNode->path, tempPathRemote, sftp))
        return;

    // substitute command

    // launch it

    // check results

    // apply changes back

}
