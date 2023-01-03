#include "Lib/SyncManager.h"

#include <sys/stat.h>
#include "Utils.h"

int SyncFileLocalToRemote(PairedNode* pNode, SSHConnector& ssh)
{
    switch (pNode->localNode.status)
    {
    case FileNode::Status::Absent:
        pNode->deleted = true;
        // delete
        return 1;
    case FileNode::Status::Clean:
    case FileNode::Status::Dirty:
    case FileNode::Status::New:
    {
        //send
        //stat local for dev/inode
        auto path = pNode->localNode.path.c_str();
        struct stat buf;
        stat(path, &buf);
        auto newLocalDev = buf.st_dev;
        auto newLocalInode = buf.st_ino;
        //stat remote for dev/inode, mtime
        if (ssh.StatRemote(pNode->remoteNode.path, &buf) != CALLCLI_OK)
        {
            //uh oh
        }
        auto newRemoteDev = buf.st_dev;
        auto newRemoteInode = buf.st_ino;
        auto remoteMtime = buf.st_mtim.tv_sec;

        pNode->remoteNode = FileNode(pNode->localNode.path, newRemoteDev, newRemoteInode,
                                     remoteMtime, pNode->localNode.size, pNode->localNode.hashHigh,
                                     pNode->localNode.hashLow);
        pNode->localNode.status = FileNode::Status::Clean;
        pNode->remoteNode.status = FileNode::Status::Clean;
        pNode->historyNode = HistoryFileNode(pNode->localNode.path, newLocalDev, newLocalInode,
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

int SyncFileRemoteToLocal(PairedNode* pNode, SSHConnector& ssh)
{


    switch (pNode->remoteNode.status)
    {
    case FileNode::Status::Absent:
        pNode->deleted = true;
        // delete
        return 1;
    case FileNode::Status::Clean:
    case FileNode::Status::Dirty:
    case FileNode::Status::New:
    {
        //send
        //stat local for dev/inode, mtime
        auto path = pNode->localNode.path.c_str();
        struct stat buf;
        stat(path, &buf);
        auto newLocalDev = buf.st_dev;
        auto newLocalInode = buf.st_ino;
        auto localMtime = buf.st_mtim.tv_sec;
        if (ssh.StatRemote(pNode->remoteNode.path, &buf) != CALLCLI_OK)
        {
            //uh oh
        }
        //stat remote for dev/inode
        auto newRemoteDev = buf.st_dev;
        auto newRemoteInode = buf.st_ino;

        pNode->localNode = FileNode(pNode->remoteNode.path, newLocalDev, newLocalInode,
                                    localMtime, pNode->remoteNode.size, pNode->remoteNode.hashHigh,
                                    pNode->remoteNode.hashLow);
        pNode->localNode.status = FileNode::Status::Clean;
        pNode->remoteNode.status = FileNode::Status::Clean;
        pNode->historyNode = HistoryFileNode(pNode->remoteNode.path, newLocalDev, newLocalInode,
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

int SyncManager::Sync(PairedNode* pNode, SSHConnector& ssh, DBConnector& db)
{
    if (pNode->action == PairedNode::Action::Ignore
        || pNode->action == PairedNode::Action::DoNothing
        || pNode->action == PairedNode::Action::Conflict
        || pNode->action == PairedNode::Action::Cancel)
        return 1;

    bool wasDeleted = false;

    switch (pNode->action)
    {
    case PairedNode::Action::LocalToRemote:
        wasDeleted = SyncFileLocalToRemote(pNode, ssh);
        break;
    case PairedNode::Action::RemoteToLocal:
        wasDeleted = SyncFileRemoteToLocal(pNode, ssh);
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

    return 0;
}
