#include "Lib/SyncManager.h"

#include <chrono>

#include "Lib/FileSystem.h"
#include "Lib/PairingManager.h"
#include "Lib/ConflictManager.h"
#include "Lib/Compressor.h"
#include "Utils.h"

std::string MakeTempPathForLocal(PairedNode* pNode)
{
    return pNode->pathHash + '-' + pNode->localNode.HashToString() + ".SyncTEMP";
}

std::string MakeTempPathForRemote(PairedNode* pNode)
{
    return pNode->pathHash + '-' + pNode->remoteNode.HashToString() + ".SyncTEMP";
}

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

void Finalize(PairedNode* pNode)
{
    pNode->localNode.status = FileNode::Status::Clean;
    pNode->remoteNode.status = FileNode::Status::Clean;
    pNode->historyNode = HistoryFileNode(pNode->path, pNode->localNode.dev, pNode->localNode.inode,
                                            pNode->remoteNode.dev, pNode->remoteNode.inode, pNode->localNode.mtime,
                                            pNode->remoteNode.mtime, pNode->localNode.size, pNode->localNode.hashHigh,
                                            pNode->localNode.hashLow);
    pNode->SetDefaultAction(PairedNode::Action::DoNothing);
}

int SyncFileLocalToRemote(PairedNode* pNode, std::string& remotePath, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp, bool& wasDeleted)
{
    switch (pNode->localNode.status)
    {
    case FileNode::Status::Absent:
        if (!sftp.Delete(remotePath))
            return -1;
        pNode->deleted = wasDeleted = true;
        pNode->localNode.status = FileNode::Status::Absent;
        pNode->remoteNode.status = FileNode::Status::Absent;
        break;
    case FileNode::Status::Clean:
    case FileNode::Status::Dirty:
    case FileNode::Status::New:
    {
        //send
        std::string tempFilePath = MakeTempPathForLocal(pNode);
        std::string tempFilePathLocal = Utils::GetTempPath() + tempFilePath;
        std::string tempFilePathRemote = tempPath + tempFilePath;

        //compress if > 50MB
        if (pNode->localNode.size > 50000000)
        {
            // if we already have the uncompressed file transfered, don't bother, just move
            sftp_attributes info;
            if ((info = sftp.Stat(tempFilePathRemote.c_str())) == nullptr || (off_t)info->size != pNode->localNode.size)
            {
                off_t compressedSize = 0;
                if (!Compressor::Compress(pNode->path, tempFilePathLocal+".zst", compressedSize))
                    return -1;
                if (!sftp.Send(tempFilePathLocal+".zst", tempFilePathRemote+".zst", compressedSize))
                    return -1;
                if (ssh.CallCLIDecompress(tempFilePathRemote+".zst", tempFilePathRemote) != CALLCLI_OK)
                    return -1;
            }
            if (ssh.ReplaceFile(tempFilePathRemote, remotePath) != CALLCLI_OK)
                return -1;
            sftp.Delete(tempFilePathRemote+".zst");
            remove((tempFilePathLocal+".zst").c_str());
        }
        else
        {
            if (!sftp.Send(pNode->path, tempFilePathRemote, pNode->localNode.size))
                return -1;
            if (ssh.ReplaceFile(tempFilePathRemote, remotePath) != CALLCLI_OK)
                return -1;
        }

        std::time_t mtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        pNode->remoteNode = FileNode(pNode->path, pNode->localNode.dev, pNode->localNode.inode,
                                     mtime, pNode->localNode.size, pNode->localNode.hashHigh,
                                     pNode->localNode.hashLow);
        Finalize(pNode);
        break;
    }
    default:
        return -1;
    }

    return 0;
}

int SyncFileRemoteToLocal(PairedNode* pNode, std::string& remotePath, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp, bool& wasDeleted)
{
    switch (pNode->remoteNode.status)
    {
    case FileNode::Status::Absent:
        if (remove(pNode->path.c_str()) != 0)
            return -1;
        pNode->deleted = wasDeleted = true;
        pNode->localNode.status = FileNode::Status::Absent;
        pNode->remoteNode.status = FileNode::Status::Absent;
        break;
    case FileNode::Status::Clean:
    case FileNode::Status::Dirty:
    case FileNode::Status::New:
    {
        //receive
        std::string tempFilePath = MakeTempPathForRemote(pNode);
        std::string tempFilePathLocal = Utils::GetTempPath() + tempFilePath;
        std::string tempFilePathRemote = tempPath + tempFilePath;

        //compress if > 50MB
        if (pNode->remoteNode.size > 50000000)
        {
            // if we already have the uncompressed file transfered, don't bother, just move
            struct stat buf;
            if (stat(tempFilePathLocal.c_str(), &buf) != 0 || (off_t)buf.st_size != pNode->remoteNode.size)
            {
                off_t compressedSize = 0;
                if (ssh.CallCLICompress(remotePath, tempFilePathRemote+".zst", &compressedSize) != CALLCLI_OK)
                    return -1;
                if (!sftp.Receive(tempFilePathRemote+".zst", tempFilePathLocal+".zst", compressedSize))
                    return -1;
                if (!Compressor::Decompress(tempFilePathLocal+".zst", tempFilePathLocal))
                    return -1;
            }
            if (!FileSystem::MoveLocalFile(tempFilePathLocal, pNode->path))
                return -1;
            sftp.Delete(tempFilePathRemote+".zst");
            remove((tempFilePathLocal+".zst").c_str());
        }
        else
        {
            if (!sftp.Receive(remotePath, tempFilePathLocal, pNode->remoteNode.size))
                return -1;
            if (!FileSystem::MoveLocalFile(tempFilePathLocal, pNode->path))
                return -1;
        }

        std::time_t mtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        pNode->localNode = FileNode(pNode->path, pNode->localNode.dev, pNode->localNode.inode,
                                    mtime, pNode->remoteNode.size, pNode->remoteNode.hashHigh,
                                    pNode->remoteNode.hashLow);
        Finalize(pNode);
        break;
    }
    default:
        return -1;
    }

    return 0;
}

int SyncResolve(PairedNode* pNode, std::string& remotePath, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp)
{
    std::string tempLocal = Utils::GetTempPath() + pNode->pathHash + ConflictManager::tempSuffixLocal;
    std::string tempRemote = Utils::GetTempPath() + pNode->pathHash + ConflictManager::tempSuffixRemote;

    FileNode local;
    FileNode remote;
    if (Creeper::MakeSingleNode(tempLocal, local) != Creeper::Result::Ok)
        return -1;
    if (Creeper::MakeSingleNode(tempRemote, remote) != Creeper::Result::Ok)
        return -1;
    // if the "resolved" files are different, cancel
    if (!local.IsEqualHash(remote))
        return -1;

    // move temp/local first
    if (!FileSystem::MoveLocalFile(tempLocal, pNode->path))
        return -1;
    //stat local for dev/inode
    auto path = pNode->path.c_str();
    struct stat buf;
    stat(path, &buf);
    auto newLocalDev = buf.st_dev;
    auto newLocalInode = buf.st_ino;
    auto newLocalMtime = buf.st_mtim.tv_sec;
    auto newLocalSize = buf.st_size;

    // move temp/remote second
    //send
    std::string tempFilePathRemote = tempPath + pNode->pathHash + ConflictManager::tempSuffixRemote;

    //compress if > 50MB
    if (pNode->localNode.size > 50000000)
    {
        // if we already have the uncompressed file transfered, don't bother, just move
        sftp_attributes info;
        if ((info = sftp.Stat(tempFilePathRemote.c_str())) == nullptr || (off_t)info->size != pNode->localNode.size)
        {
            off_t compressedSize = 0;
            if (!Compressor::Compress(tempRemote, tempRemote+".zst", compressedSize))
                return -1;
            if (!sftp.Send(tempRemote+".zst", tempFilePathRemote+".zst", compressedSize))
                return -1;
            if (ssh.CallCLIDecompress(tempFilePathRemote+".zst", tempFilePathRemote) != CALLCLI_OK)
                return -1;
        }
        if (ssh.ReplaceFile(tempFilePathRemote, remotePath) != CALLCLI_OK)
            return -1;
        sftp.Delete(tempFilePathRemote+".zst");
        remove((tempRemote+".zst").c_str());
        remove(tempRemote.c_str());
    }
    else
    {
        if (!sftp.Send(tempRemote, tempFilePathRemote, pNode->localNode.size))
            return -1;
        if (ssh.ReplaceFile(tempFilePathRemote, remotePath) != CALLCLI_OK)
            return -1;
        remove(tempRemote.c_str());
    }

    //stat remote for dev/inode, mtime
    if (ssh.StatRemote(remotePath, &buf) != CALLCLI_OK)
    {
        return -1;
    }
    auto newRemoteDev = buf.st_dev;
    auto newRemoteInode = buf.st_ino;
    auto newRemoteMtime = buf.st_mtim.tv_sec;
    auto newRemoteSize = buf.st_size;
    
    pNode->localNode = FileNode(pNode->path, newLocalDev, newLocalInode,
                                newLocalMtime, newLocalSize, local.hashHigh,
                                local.hashLow);
    pNode->remoteNode = FileNode(pNode->path, newRemoteDev, newRemoteInode,
                                 newRemoteMtime, newRemoteSize, remote.hashHigh,
                                 remote.hashLow);
    Finalize(pNode);

    return 0;
}

bool LastMinuteCheck(PairedNode* pNode, std::string& remotePath, SFTPConnector& sftp)
{
    if (pNode->progress == PairedNode::Progress::Canceled)
        return false;

    FileNode local;
    FileNode remote;
    switch(Creeper::MakeSingleNodeLight(pNode->path, local))
    {
    case Creeper::Result::Ok:
        local.status = FileNode::Status::Changed;
        break;
    case Creeper::Result::NotExists:
        break;
    default:
        pNode->progress = PairedNode::Progress::Failed;
        return false;
    }

    sftp_attributes info;
    if ((info = sftp.Stat(remotePath)) == nullptr)
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

int SyncManager::Sync(PairedNode* pNode, std::string& remoteRoot, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp, HistoryFileNodeDBConnector& db)
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
        if (SyncFileRemoteToLocal(pNode, remotePath, tempPath, ssh, sftp, wasDeleted) != 0)
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
        break;
    case PairedNode::Action::DoNothing:
        return 0;
    case PairedNode::Action::FastForward:
        UpdateHistory(pNode, wasDeleted);
        break;
    case PairedNode::Action::Resolve:
        if (SyncResolve(pNode, remotePath, tempPath, ssh, sftp) != 0)
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
        break;
    default:
        break;
    }

    if (wasDeleted)
    {
        if (!db.Delete(pNode->path))
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
    }
    else
    {
        if (!db.Update(pNode->historyNode))
        {
            pNode->progress = PairedNode::Progress::Failed;
            return 2;
        }
    }

    pNode->progress = PairedNode::Progress::Success;

    return 0;
}
