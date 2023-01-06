#include "Lib/SyncManager.h"

#include "Lib/FileSystem.h"
#include "Lib/PairingManager.h"
#include "Lib/ConflictManager.h"
#include "Utils.h"

int UpdateHistory(PairedNode* pNode, bool& wasDeleted)
{
    if (pNode->localNode.status == FileNode::Status::Absent)
    {
        pNode->deleted = wasDeleted = true;
        return 0;
    }

    pNode->localNode.status = FileNode::Status::Clean;
    pNode->remoteNode.status = FileNode::Status::Clean;
    pNode->historyNode = HistoryFileNode(pNode->path, pNode->localNode.dev, pNode->localNode.inode,
                                         pNode->remoteNode.dev, pNode->remoteNode.inode, pNode->localNode.mtime,
                                         pNode->remoteNode.mtime, pNode->localNode.size, pNode->localNode.hashHigh,
                                         pNode->localNode.hashLow);
    pNode->SetDefaultAction(PairedNode::Action::DoNothing);
    return 0;
}

int SyncFileLocalToRemote(PairedNode* pNode, std::string& remotePath, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp, bool& wasDeleted)
{
    switch (pNode->localNode.status)
    {
    case FileNode::Status::Absent:
        if (!sftp.Delete(remotePath))
            return -1;
        pNode->deleted = wasDeleted = true;
        break;
    case FileNode::Status::Clean:
    case FileNode::Status::Dirty:
    case FileNode::Status::New:
    {
        //send
        std::string tempFilePath = pNode->pathHash + '-' + pNode->localNode.HashToString();
        if (!sftp.Send(pNode->path, remotePath, tempPath, tempFilePath, pNode->localNode.size))
            return -1;
        if (ssh.ReplaceFile(tempFilePath, remotePath) != CALLCLI_OK)
            return -1;
        //stat local for dev/inode
        auto path = pNode->path.c_str();
        struct stat buf;
        stat(path, &buf);
        auto newLocalDev = buf.st_dev;
        auto newLocalInode = buf.st_ino;
        //stat remote for dev/inode, mtime
        if (ssh.StatRemote(remotePath, &buf) != CALLCLI_OK)
        {
            //uh oh
            return -1;
        }
        auto newRemoteDev = buf.st_dev;
        auto newRemoteInode = buf.st_ino;
        auto remoteMtime = buf.st_mtim.tv_sec;

        pNode->remoteNode = FileNode(pNode->path, newRemoteDev, newRemoteInode,
                                     remoteMtime, pNode->localNode.size, pNode->localNode.hashHigh,
                                     pNode->localNode.hashLow);
        pNode->localNode.status = FileNode::Status::Clean;
        pNode->remoteNode.status = FileNode::Status::Clean;
        pNode->historyNode = HistoryFileNode(pNode->path, newLocalDev, newLocalInode,
                                             newRemoteDev, newRemoteInode, pNode->localNode.mtime,
                                             remoteMtime, pNode->localNode.size, pNode->localNode.hashHigh,
                                             pNode->localNode.hashLow);
        pNode->SetDefaultAction(PairedNode::Action::DoNothing);
        break;
    }
    default:
        break;
    }

    return 0;
}

int SyncFileRemoteToLocal(PairedNode* pNode, std::string& remotePath, SSHConnector& ssh, SFTPConnector& sftp, bool& wasDeleted)
{
    switch (pNode->remoteNode.status)
    {
    case FileNode::Status::Absent:
        if (remove(pNode->path.c_str()) != 0)
            return -1;
        pNode->deleted = wasDeleted = true;
        break;
    case FileNode::Status::Clean:
    case FileNode::Status::Dirty:
    case FileNode::Status::New:
    {
        //receive
        std::string tempFilePath = pNode->pathHash + '-' + pNode->remoteNode.HashToString();
        if (!sftp.Receive(pNode->path, remotePath, tempFilePath, pNode->remoteNode.size))
            return -1;
        //stat local for dev/inode, mtime
        auto path = pNode->path.c_str();
        struct stat buf;
        stat(path, &buf);
        auto newLocalDev = buf.st_dev;
        auto newLocalInode = buf.st_ino;
        auto newLocalMtime = buf.st_mtim.tv_sec;
        //stat remote for dev/inode
        if (ssh.StatRemote(remotePath, &buf) != CALLCLI_OK)
        {
            //uh oh
            return -1;
        }
        auto newRemoteDev = buf.st_dev;
        auto newRemoteInode = buf.st_ino;

        pNode->localNode = FileNode(pNode->path, newLocalDev, newLocalInode,
                                    newLocalMtime, pNode->remoteNode.size, pNode->remoteNode.hashHigh,
                                    pNode->remoteNode.hashLow);
        pNode->localNode.status = FileNode::Status::Clean;
        pNode->remoteNode.status = FileNode::Status::Clean;
        pNode->historyNode = HistoryFileNode(pNode->path, newLocalDev, newLocalInode,
                                             newRemoteDev, newRemoteInode, newLocalMtime, pNode->remoteNode.mtime,
                                             pNode->remoteNode.size, pNode->remoteNode.hashHigh,
                                             pNode->remoteNode.hashLow);
        pNode->SetDefaultAction(PairedNode::Action::DoNothing);
        break;
    }
    default:
        break;
    }

    return 0;
}

int SyncResolve(PairedNode* pNode, std::string& remotePath, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp)
{
    std::string tempLocal = Utils::GetTempPath() + pNode->pathHash + ConflictManager::tempSuffixLocal;
    std::string tempRemote = Utils::GetTempPath() + pNode->pathHash + ConflictManager::tempSuffixRemote;

    FileNode local;
    FileNode remote;
    if (Creeper::MakeSingleNode(tempLocal, local) != CREEP_OK)
        return -1;
    if (Creeper::MakeSingleNode(tempRemote, remote) != CREEP_OK)
        return -1;
    if (!local.IsEqualHash(remote))
        return -1;

    // move temp/local first
    // try to move atomically, if it fails, copy the old fashioned way
    if (rename(tempLocal.c_str(), pNode->path.c_str()) < 0)
    {
        // error!
        int err = errno;
        LOG(WARNING) << "Error moving temporary file " << tempLocal << " atomically. Message: " << strerror(err);
        if (err == EXDEV)
        {
            if (FileSystem::CopyLocalFile(tempLocal, pNode->path, std::filesystem::copy_options::overwrite_existing))
                remove(tempLocal.c_str());
            else
                return -1;
        }
        else
        {
            return -1;
        }
    }
    //stat local for dev/inode
    auto path = pNode->path.c_str();
    struct stat buf;
    stat(path, &buf);
    auto newLocalDev = buf.st_dev;
    auto newLocalInode = buf.st_ino;
    auto newLocalMtime = buf.st_mtim.tv_sec;
    auto newLocalSize = buf.st_size;

    // then move temp/remote
    //send
    std::string tempFilePath = pNode->pathHash + '-' + pNode->remoteNode.HashToString();
    if (!sftp.Send(tempRemote, remotePath, tempPath, tempRemote, pNode->localNode.size))
        return -1;
    if (ssh.ReplaceFile(tempRemote, remotePath) != CALLCLI_OK)
        return -1;
    remove(tempRemote.c_str());
    //stat remote for dev/inode, mtime
    if (ssh.StatRemote(remotePath, &buf) != CALLCLI_OK)
    {
        //uh oh
        return -1;
    }
    auto newRemoteDev = buf.st_dev;
    auto newRemoteInode = buf.st_ino;
    auto newRemoteMtime = buf.st_mtim.tv_sec;
    auto newRemoteSize = buf.st_size;

    // resolve history
    pNode->localNode = FileNode(pNode->path, newLocalDev, newLocalInode,
                                newLocalMtime, newLocalSize, local.hashHigh,
                                local.hashLow);
    pNode->remoteNode = FileNode(pNode->path, newRemoteDev, newRemoteInode,
                                 newRemoteMtime, newRemoteSize, remote.hashHigh,
                                 remote.hashLow);
    pNode->historyNode = HistoryFileNode(pNode->path, newLocalDev, newLocalInode,
                                         newRemoteDev, newRemoteInode, newLocalMtime,
                                         newRemoteMtime, newLocalSize, pNode->localNode.hashHigh,
                                         pNode->localNode.hashLow);
    pNode->localNode.status = FileNode::Status::Clean;
    pNode->remoteNode.status = FileNode::Status::Clean;
    pNode->SetDefaultAction(PairedNode::Action::DoNothing);

    return 0;
}

bool LastMinuteCheck(PairedNode* pNode, std::string& remotePath, SFTPConnector& sftp)
{
    FileNode local;
    FileNode remote;
    switch(Creeper::MakeSingleNodeLight(pNode->path, local))
    {
    case CREEP_OK:
        local.status = FileNode::Status::Changed;
        break;
    case CREEP_EXIST:
        break;
    default:
        pNode->progress = PairedNode::Progress::Failed;
        return false;
    }

    sftp_attributes info;
    if ((info = sftp.Stat(remotePath)) == NULL)
    {
        if (!sftp.IsAbsent())
        {
            pNode->progress = PairedNode::Progress::Failed;
            return false;
        }
    }
    else
    {
        remote.status = FileNode::Status::Changed;
        remote.size = info->size;
        remote.mtime = info->mtime;
    }
    
    if (!PairingManager::CheckChanges(pNode->localNode, local, pNode->remoteNode, remote))
    {
        pNode->progress = PairedNode::Progress::Canceled;
        pNode->action = PairedNode::Action::Ignore;
        sftp_attributes_free(info);
        return false;
    }
    sftp_attributes_free(info);

    return true;
}

int SyncManager::Sync(PairedNode* pNode, std::string& remoteRoot, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp, DBConnector& db)
{
    if (pNode->action == PairedNode::Action::Ignore
        || pNode->action == PairedNode::Action::DoNothing
        || pNode->action == PairedNode::Action::Conflict)
        return 1;

    std::string remotePath = remoteRoot + pNode->path;

    // check for cancel here
    if (!LastMinuteCheck(pNode, remotePath, sftp))
        return 3;

    bool wasDeleted = false;
    switch (pNode->action)
    {
    case PairedNode::Action::LocalToRemote:
        if (SyncFileLocalToRemote(pNode, remotePath, tempPath, ssh, sftp, wasDeleted) != 0)
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
        break;
    case PairedNode::Action::RemoteToLocal:
        if (SyncFileRemoteToLocal(pNode, remotePath, ssh, sftp, wasDeleted) != 0)
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
        break;
    case PairedNode::Action::DoNothing:
        return 0;
    case PairedNode::Action::FastForward:
        UpdateHistory(pNode, wasDeleted);
    case PairedNode::Action::Resolve:
        if (SyncResolve(pNode, remotePath, tempPath, ssh, sftp) != 0)
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
    default:
        break;
    }

    if (wasDeleted)
        db.DeleteFileNode(pNode->path);
    else
        db.UpdateFileNode(pNode->historyNode);

    pNode->progress = PairedNode::Progress::Success;

    return 0;
}
