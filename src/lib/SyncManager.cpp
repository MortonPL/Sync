#include "Lib/SyncManager.h"

#include "xxhash.h"
#include <sys/stat.h>
#include "Utils.h"

std::string QuickHash(std::string value)
{
    return fmt::format("{:x}", XXH64(value.c_str(), value.size(), 0));
}

int SyncFileLocalToRemote(PairedNode* pNode, std::string& remoteRoot, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp, bool& wasDeleted)
{
    std::string fullPath = remoteRoot + pNode->path;
    std::string hashedPath = QuickHash(pNode->path);
    switch (pNode->localNode.status)
    {
    case FileNode::Status::Absent:
        if (!sftp.Delete(fullPath))
            return -1;
        pNode->deleted = wasDeleted = true;
        break;
    case FileNode::Status::Clean:
    case FileNode::Status::Dirty:
    case FileNode::Status::New:
    {
        //send
        if (!sftp.Send(pNode->path, fullPath, tempPath, hashedPath, pNode->localNode.size))
            return -1;
        if (ssh.ReplaceFile(hashedPath, fullPath) != CALLCLI_OK)
            return -1;
        //stat local for dev/inode
        auto path = pNode->path.c_str();
        struct stat buf;
        stat(path, &buf);
        auto newLocalDev = buf.st_dev;
        auto newLocalInode = buf.st_ino;
        //stat remote for dev/inode, mtime
        if (ssh.StatRemote(fullPath, &buf) != CALLCLI_OK)
        {
            //uh oh
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

int SyncFileRemoteToLocal(PairedNode* pNode, std::string& remoteRoot, SSHConnector& ssh, SFTPConnector& sftp, bool& wasDeleted)
{
    std::string fullPath = remoteRoot + pNode->path;
    std::string hashedPath = QuickHash(pNode->path);
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
        if (!sftp.Receive(pNode->path, fullPath, hashedPath, pNode->remoteNode.size))
            return -1;
        //stat local for dev/inode, mtime
        auto path = pNode->path.c_str();
        struct stat buf;
        stat(path, &buf);
        auto newLocalDev = buf.st_dev;
        auto newLocalInode = buf.st_ino;
        auto localMtime = buf.st_mtim.tv_sec;
        //stat remote for dev/inode
        if (ssh.StatRemote(fullPath, &buf) != CALLCLI_OK)
        {
            //uh oh
        }
        auto newRemoteDev = buf.st_dev;
        auto newRemoteInode = buf.st_ino;

        pNode->localNode = FileNode(pNode->path, newLocalDev, newLocalInode,
                                    localMtime, pNode->remoteNode.size, pNode->remoteNode.hashHigh,
                                    pNode->remoteNode.hashLow);
        pNode->localNode.status = FileNode::Status::Clean;
        pNode->remoteNode.status = FileNode::Status::Clean;
        pNode->historyNode = HistoryFileNode(pNode->path, newLocalDev, newLocalInode,
                                             newRemoteDev, newRemoteInode, localMtime, pNode->remoteNode.mtime,
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

int SyncManager::Sync(PairedNode* pNode, std::string& remoteRoot, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp, DBConnector& db)
{
    if (pNode->progress == PairedNode::Progress::Canceled)
        return 1;

    if (pNode->action == PairedNode::Action::Ignore
        || pNode->action == PairedNode::Action::DoNothing
        || pNode->action == PairedNode::Action::Conflict)
        return 1;

    bool wasDeleted = false;

    switch (pNode->action)
    {
    case PairedNode::Action::LocalToRemote:
        if (SyncFileLocalToRemote(pNode, remoteRoot, tempPath, ssh, sftp, wasDeleted) != 0)
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
        break;
    case PairedNode::Action::RemoteToLocal:
        if (SyncFileRemoteToLocal(pNode, remoteRoot, ssh, sftp, wasDeleted) != 0)
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
        break;
    case PairedNode::Action::DoNothing:
    case PairedNode::Action::FastForward:
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
