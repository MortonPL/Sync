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
    }
}

void PairingManager::PairAllHistory(std::forward_list<HistoryFileNode>& historyNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper)
{
    for (auto& historyNode: historyNodes)
    {
        auto pPair = mapper.FindMapPath(historyNode.path);
        if (pPair)
        {
            if (historyNode.IsEqualHash(pPair->localNode))
            {
                pPair->localNode.status = FileNode::Status::Clean;
            }
            else
            {
                pPair->localNode.status = FileNode::Status::Dirty;
            }
        }
        else
        {
            pairedNodes.push_back(PairedNode(historyNode.path));
            pPair = &pairedNodes.back();
        }
        pPair->historyNode = historyNode;
        mapper.EmplaceMapPath(historyNode.path, *pPair);
    }
}

void PairRemoteCompare(FileNode& remoteNode, PairedNode* pPair)
{
    switch (pPair->CompareNodeHashes(remoteNode, pPair->historyNode))
    {
    case PairedNode::HashComparisonResult::Equal:
        remoteNode.status = FileNode::Status::Clean;
        break;
    case PairedNode::HashComparisonResult::OtherNull:
        remoteNode.status = FileNode::Status::New;
        if (pPair->CompareNodeHashes(remoteNode, pPair->localNode) == PairedNode::HashComparisonResult::Equal)
        {
            pPair->SetDefaultAction(PairedNode::Action::FastForward);
        }
        else
        {
            pPair->SetDefaultAction(PairedNode::Action::Conflict);
        }
        break;
    case PairedNode::HashComparisonResult::NotEqual:
        remoteNode.status = FileNode::Status::Dirty;
        if (pPair->CompareNodeHashes(remoteNode, pPair->localNode) == PairedNode::HashComparisonResult::Equal)
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
    default:
        break;
    }
}

void PairingManager::PairAllRemote(std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper, Mapper& mapper)
{
    for(auto& remoteNode: remoteNodes)
    {
        if (creeper.CheckIfFileIsIgnored(remoteNode.path))
            continue;

        auto pPair = mapper.FindMapPath(remoteNode.path);
        if (pPair)
        {
            PairRemoteCompare(remoteNode, pPair);
        }
        else
        {
            pairedNodes.push_back(PairedNode(remoteNode.path));
            pPair = &pairedNodes.back();
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
            pair.SetDefaultAction(PairedNode::Action::FastForward);
        }
        else if (pair.localNode.status == FileNode::Status::New)
        {
            pair.SetDefaultAction(PairedNode::Action::LocalToRemote);
        }
        else
        {
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
    PairingManager::PairAllLocal(scanNodes, pairedNodes, mapper);
    PairingManager::PairAllHistory(historyNodes, pairedNodes, mapper);
    PairingManager::PairAllRemote(remoteNodes, pairedNodes, creeper, mapper);
    PairingManager::SolveFinalAction(pairedNodes);
}

bool PairingManager::CheckChanges(FileNode& localOld, FileNode& localNew, FileNode& remoteOld, FileNode& remoteNew)
{
    bool isOk = true;

    if (localOld.size != localNew.size || localOld.mtime != localNew.mtime)
    {
        localOld = localNew;
        isOk = false;
    }

    if (remoteOld.size != remoteNew.size || remoteOld.mtime != remoteNew.mtime)
    {
        remoteOld = remoteNew;
        isOk = false;
    }

    return isOk;
}
