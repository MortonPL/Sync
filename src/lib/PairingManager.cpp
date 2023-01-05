#include "Lib/PairingManager.h"

#include "Utils.h"

void PairingManager::PairAllLocal(std::forward_list<FileNode>& scanNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper)
{
    for (auto& scanNode: scanNodes)
    {
        auto pair = PairedNode(scanNode.path);
        pair.localNode = scanNode;
        pairedNodes.push_back(pair);
        mapper.EmplaceMapPath(scanNode.path, pairedNodes.back());
        mapper.EmplaceMapLocalInode(scanNode.GetDevInode(), pairedNodes.back());
    }
}

void PairHistoryNotMoved(HistoryFileNode& historyNode, PairedNode* pPair)
{
    if (historyNode.IsEqualHash((*pPair).localNode))
    {
        pPair->localNode.status = FileNode::Status::Clean;
    }
    else
    {
        pPair->localNode.status = FileNode::Status::Dirty;
    }
};

void PairHistoryMoved(HistoryFileNode& historyNode, PairedNode* pPair)
{
    if (historyNode.IsEqualHash((*pPair).localNode))
    {
        pPair->localNode.status = FileNode::Status::MovedClean;
    }
    else
    {
        pPair->localNode.status = FileNode::Status::MovedDirty;
    }
}

void PairingManager::PairAllHistory(std::forward_list<HistoryFileNode>& historyNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper)
{
    for (auto& historyNode: historyNodes)
    {
        auto pPair = mapper.FindMapLocalInode(historyNode.GetDevInode());
        if (pPair)
        {
            if (historyNode.path == pPair->localNode.path)
            {
                PairHistoryNotMoved(historyNode, pPair);
            }
            else
            {
                PairHistoryMoved(historyNode, pPair);
            }
        }
        else
        {
            pPair = mapper.FindMapPath(historyNode.path);
            if (pPair)
            {
                PairHistoryNotMoved(historyNode, pPair);
            }
            else
            {
                historyNode.status = FileNode::Status::DeletedLocal;
                pairedNodes.push_back(PairedNode(historyNode.path));
                pPair = &pairedNodes.back();
            }
        }
        pPair->historyNode = historyNode;
        mapper.EmplaceMapPath(historyNode.path, *pPair);
        mapper.EmplaceMapRemoteInode(historyNode.GetRemoteDevInode(), *pPair);
    }
}

void PairRemoteNotMoved(FileNode& remoteNode, PairedNode* pPair)
{
    switch (pPair->CompareNodeHashes(remoteNode, pPair->historyNode))
    {
    case HASHCMP_EQ:
        remoteNode.status = FileNode::Status::Clean;
        break;
    case HASHCMP_OTHERNULL:
        remoteNode.status = FileNode::Status::New;
        if (pPair->CompareNodeHashes(remoteNode, pPair->localNode) == HASHCMP_EQ)
        {
            pPair->SetDefaultAction(PairedNode::Action::FastForward);
        }
        else
        {
            pPair->SetDefaultAction(PairedNode::Action::Conflict);
        }
        break;
    case HASHCMP_NE:
    default:
        remoteNode.status = FileNode::Status::Dirty;
        if (pPair->CompareNodeHashes(remoteNode, pPair->localNode) == HASHCMP_EQ)
        {
            if (remoteNode.path == pPair->localNode.path)
            {
                pPair->SetDefaultAction(PairedNode::Action::FastForward);
            }
            else
            {
                pPair->SetDefaultAction(PairedNode::Action::Conflict);
            }
        }
        else
        {
            if (pPair->localNode.status == FileNode::Status::Clean)
            {
            }
            else
            {
                pPair->SetDefaultAction(PairedNode::Action::Conflict);
            }
        }
        break;
    }
};

void PairRemoteMovedSub(FileNode& remoteNode, PairedNode* pPair, FileNode::Status status)
{
    remoteNode.status = status;
    if (pPair->CompareNodeHashes(remoteNode, pPair->localNode) == HASHCMP_EQ)
    {
        if (remoteNode.path == pPair->localNode.path)
        {
            pPair->SetDefaultAction(PairedNode::Action::FastForward);
        }
        else
        {
            if (pPair->localNode.status == FileNode::Status::Clean)
            {
            }
            else
            {
                pPair->SetDefaultAction(PairedNode::Action::Conflict);
            }
        }
    }
    else
    {
        if (pPair->localNode.status == FileNode::Status::Clean)
        {
        }
        else
        {
            pPair->SetDefaultAction(PairedNode::Action::Conflict);
        }
    }
}

void PairRemoteMoved(FileNode& remoteNode, PairedNode* pPair)
{
    if (pPair->CompareNodeHashes(remoteNode, pPair->historyNode) == HASHCMP_EQ)
    {
        PairRemoteMovedSub(remoteNode, pPair, FileNode::Status::MovedClean);
    }
    else
    {
        PairRemoteMovedSub(remoteNode, pPair, FileNode::Status::MovedDirty);
    }
}

void PairingManager::PairAllRemote(std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper, Mapper& mapper)
{
    for(auto& remoteNode: remoteNodes)
    {
        if (creeper.CheckIfFileIsIgnored(remoteNode.path))
            continue;
        //auto pPair = mapper.FindMapPath(remoteNode.path);
        auto pPair = mapper.FindMapRemoteInode(remoteNode.GetDevInode());
        if (pPair)
        {
            if (remoteNode.path == pPair->historyNode.path)
            {
                PairRemoteNotMoved(remoteNode, pPair);
            }
            else
            {
                PairRemoteMoved(remoteNode, pPair);
            }
        }
        else
        {
            pPair = mapper.FindMapPath(remoteNode.path);
            if (pPair)
            {
                PairRemoteNotMoved(remoteNode, pPair);
            }
            else
            {
                pairedNodes.push_back(PairedNode(remoteNode.path));
                pPair = &pairedNodes.back();
            }
        }
        pPair->remoteNode = remoteNode;
    }
}

void PairingManager::SolveFinalAction(std::list<PairedNode>& pairedNodes)
{
    for (auto& pair: pairedNodes)
    {
        pair.pathHash = Utils::QuickHash(pair.path);

        if (pair.action == PairedNode::Action::FastForward || pair.action == PairedNode::Action::Conflict)
            continue;
        
        if (!pair.remoteNode.IsEmpty())
        {
            if (pair.remoteNode.status == FileNode::Status::New)
            {
                pair.SetDefaultAction(PairedNode::Action::RemoteToLocal);
            }
            else
            {
                if (pair.localNode.status == FileNode::Status::Clean)
                {
                    if (pair.remoteNode.status == FileNode::Status::Clean)
                    {
                        pair.SetDefaultAction(PairedNode::Action::DoNothing);
                    }
                    else
                    {
                        pair.SetDefaultAction(PairedNode::Action::RemoteToLocal);
                    }
                }
                else if (!pair.localNode.IsEmpty())
                {
                    pair.SetDefaultAction(PairedNode::Action::LocalToRemote);
                }
                else if (!pair.historyNode.IsEmpty())
                {
                    pair.SetDefaultAction(PairedNode::Action::LocalToRemote);
                }
            }
            continue;
        }

        if (pair.localNode.IsEmpty())
        {
            pair.historyNode.status = FileNode::Status::DeletedBoth;
            pair.SetDefaultAction(PairedNode::Action::FastForward);
        }
        else if (pair.localNode.status == FileNode::Status::New)
        {
            pair.SetDefaultAction(PairedNode::Action::LocalToRemote);
        }
        else
        {
            pair.historyNode.status = FileNode::Status::DeletedRemote;
            pair.SetDefaultAction(
                pair.localNode.status == FileNode::Status::Clean?
                PairedNode::Action::RemoteToLocal:
                PairedNode::Action::Conflict);
        }
    }
}

void PairingManager::PairAll(std::forward_list<FileNode>& scanNodes, std::forward_list<HistoryFileNode>& historyNodes,
                             std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper, Mapper& mapper)
{
    //pair local
    PairingManager::PairAllLocal(scanNodes, pairedNodes, mapper);

    //pair history
    PairingManager::PairAllHistory(historyNodes, pairedNodes, mapper);

    //pair remote
    PairingManager::PairAllRemote(remoteNodes, pairedNodes, creeper, mapper);

    //final reconciliation
    PairingManager::SolveFinalAction(pairedNodes);
}
