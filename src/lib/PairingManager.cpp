#include "Lib/PairingManager.h"

void PairingManager::PairAllLocal(std::forward_list<FileNode>& scanNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper)
{
    for (auto& scanNode: scanNodes)
    {
        pairedNodes.push_back(PairedNode(scanNode.path, &scanNode));
        mapper.EmplaceMapPath(scanNode.path, pairedNodes.back());
        mapper.EmplaceMapLocalInode(scanNode.GetDevInode(), pairedNodes.back());
    }
}

void PairHistoryNotMoved(HistoryFileNode& historyNode, PairedNode* pPair)
{
    if (historyNode.IsEqualHash(*pPair->localNode))
    {
        pPair->localNode->status = FileNode::Status::Clean;
    }
    else
    {
        pPair->localNode->status = FileNode::Status::Dirty;
    }
};

void PairHistoryMoved(HistoryFileNode& historyNode, PairedNode* pPair)
{
    if (historyNode.IsEqualHash(*pPair->localNode))
    {
        pPair->localNode->status = FileNode::Status::MovedClean;
    }
    else
    {
        pPair->localNode->status = FileNode::Status::MovedDirty;
    }
}

void PairingManager::PairAllHistory(std::forward_list<HistoryFileNode>& historyNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper)
{
    for (auto& historyNode: historyNodes)
    {
        auto pPair = mapper.FindMapLocalInode(historyNode.GetDevInode());
        if (pPair)
        {
            if (historyNode.path == pPair->localNode->path)
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
                pairedNodes.push_back(PairedNode(historyNode.path, nullptr, &historyNode));
                pPair = &pairedNodes.back();
                pPair->historyNode->status = FileNode::Status::DeletedLocal;
            }
        }
        pPair->historyNode = &historyNode;
        mapper.EmplaceMapPath(historyNode.path, *pPair);
        mapper.EmplaceMapRemoteInode(historyNode.GetRemoteDevInode(), *pPair);
    }
}

void PairRemoteNotMoved(FileNode& remoteNode, PairedNode* pPair)
{
    switch (pPair->CompareNodeHashes(&remoteNode, pPair->historyNode))
    {
    case HASHCMP_EQ:
        remoteNode.status = FileNode::Status::Clean;
        break;
    case HASHCMP_OTHERNULL:
        remoteNode.status = FileNode::Status::New;
        if (pPair->CompareNodeHashes(&remoteNode, pPair->localNode) == HASHCMP_EQ)
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
        if (pPair->CompareNodeHashes(&remoteNode, pPair->localNode) == HASHCMP_EQ)
        {
            if (remoteNode.path == pPair->localNode->path)
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
            if (pPair->localNode && pPair->localNode->status == FileNode::Status::Clean)
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
    if (pPair->CompareNodeHashes(&remoteNode, pPair->localNode) == HASHCMP_EQ)
    {
        if (remoteNode.path == pPair->localNode->path)
        {
            pPair->action = PairedNode::Action::FastForward;
        }
        else
        {
            if (pPair->localNode && pPair->localNode->status == FileNode::Status::Clean)
            {
            }
            else
            {
                pPair->action = PairedNode::Action::Conflict;
            }
        }
    }
    else
    {
        if (pPair->localNode && pPair->localNode->status == FileNode::Status::Clean)
        {
        }
        else
        {
            pPair->action = PairedNode::Action::Conflict;
        }
    }
}

void PairRemoteMoved(FileNode& remoteNode, PairedNode* pPair)
{
    if (pPair->CompareNodeHashes(&remoteNode, pPair->historyNode) == HASHCMP_EQ)
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
            if (remoteNode.path == pPair->historyNode->path)
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
                pairedNodes.push_back(PairedNode(remoteNode.path, nullptr, nullptr, &remoteNode));
                pPair = &pairedNodes.back();
            }
        }
        pPair->remoteNode = &remoteNode;
    }
}

void PairingManager::SolveFinalAction(std::list<PairedNode>& pairedNodes)
{
    for (auto& pair: pairedNodes)
    {
        if (pair.action == PairedNode::Action::FastForward
            || pair.action == PairedNode::Action::Conflict)
            continue;
        
        if (pair.remoteNode)
        {
            if (pair.remoteNode->status == FileNode::Status::New)
            {
                pair.action = PairedNode::Action::RemoteToLocal;
            }
            else
            {
                if (pair.localNode && pair.localNode->status == FileNode::Status::Clean)
                {
                    if (pair.remoteNode->status == FileNode::Status::Clean)
                    {
                        pair.action = PairedNode::Action::DoNothing;
                    }
                    else
                    {
                        pair.action = PairedNode::Action::RemoteToLocal;
                    }
                }
                else if (pair.localNode)
                {
                    pair.action = PairedNode::Action::LocalToRemote;
                }
                else if (pair.historyNode)
                {
                    pair.action = PairedNode::Action::LocalToRemote;
                }
            }
            continue;
        }

        if (!pair.localNode)
        {
            pair.historyNode->status = FileNode::Status::DeletedBoth;
            pair.action = PairedNode::Action::FastForward;
        }
        else if (pair.localNode->status == FileNode::Status::New)
        {
            pair.action = PairedNode::Action::LocalToRemote;
        }
        else
        {
            pair.historyNode->status = FileNode::Status::DeletedRemote;
            pair.action = pair.localNode->status == FileNode::Status::Clean? PairedNode::Action::RemoteToLocal: PairedNode::Action::Conflict;
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
