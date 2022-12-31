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

void PairingManager::PairAllHistory(std::forward_list<HistoryFileNode>& historyNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper)
{
    for (auto& historyNode: historyNodes)
    {
        auto pPair = mapper.FindMapPath(historyNode.path);
        if (pPair)
        {
            pPair->historyNode = &historyNode;
            mapper.EmplaceMapLocalInode(historyNode.GetDevInode(), *pPair);
            mapper.EmplaceMapRemoteInode(historyNode.GetRemoteDevInode(), *pPair);
            if (historyNode.IsEqualHash(*pPair->localNode))
            {
                pPair->localNode->status = FileNode::Status::Clean;
            }
            else
            {
                pPair->localNode->status = FileNode::Status::Dirty;
            }
        }
        else
        {
            pPair = mapper.FindMapLocalInode(historyNode.GetDevInode());
            if (pPair)
            {
                pPair->historyNode = &historyNode;
                mapper.EmplaceMapPath(historyNode.path, *pPair);
                mapper.EmplaceMapRemoteInode(historyNode.GetRemoteDevInode(), *pPair);
                if (historyNode.IsEqualHash(*pPair->localNode))
                {
                    pPair->localNode->status = FileNode::Status::MovedClean;
                }
                else
                {
                    pPair->localNode->status = FileNode::Status::MovedDirty;
                }
            }
            else
            {
                pairedNodes.push_back(PairedNode(historyNode.path, nullptr, &historyNode));
                mapper.EmplaceMapPath(historyNode.path, *pPair);
                mapper.EmplaceMapRemoteInode(historyNode.GetRemoteDevInode(), *pPair);
                pPair->historyNode->status = FileNode::Status::Deleted;
            }
        }
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
            pPair->remoteNode = &remoteNode;
            switch (pPair->CompareNodeHashes(&remoteNode, pPair->historyNode))
            {
            case HASHCMP_EQ:
                pPair->remoteNode->status = FileNode::Status::Clean;
                break;
            case HASHCMP_OTHERNULL:
                pPair->remoteNode->status = FileNode::Status::New;
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
                pPair->remoteNode->status = FileNode::Status::Dirty;
                if (pPair->CompareNodeHashes(&remoteNode, pPair->localNode) == HASHCMP_EQ)
                {
                    pPair->SetDefaultAction(PairedNode::Action::FastForward);
                }
                else
                {
                    if (pPair->localNode->status != FileNode::Status::Clean)
                    {
                        pPair->SetDefaultAction(PairedNode::Action::Conflict);
                    }
                }
                break;
            case HASHCMP_ONENULL:
            case HASHCMP_EQPTR:
            default:
                break;
            }
        }
        else
        {
            pPair = mapper.FindMapRemoteInode(remoteNode.GetDevInode());
            if (pPair)
            {
                pPair->remoteNode = &remoteNode;
                if (pPair->CompareNodeHashes(&remoteNode, pPair->historyNode))
                {
                    pPair->remoteNode->status = FileNode::Status::MovedClean;
                    switch (pPair->CompareNodeHashes(&remoteNode, pPair->localNode))
                    {
                    case HASHCMP_EQ:
                        if (pPair->localNode->status != FileNode::Status::Clean)
                        {
                            pPair->SetDefaultAction(PairedNode::Action::Conflict);
                        }
                        break;
                    case HASHCMP_NE:
                    case HASHCMP_OTHERNULL:
                        pPair->SetDefaultAction(PairedNode::Action::Conflict);
                        break;
                    case HASHCMP_ONENULL:
                    case HASHCMP_EQPTR:
                    default:
                        break;
                    }
                }
                else
                {
                    switch (pPair->CompareNodeHashes(&remoteNode, pPair->localNode))
                    {
                    case HASHCMP_EQ:
                        pPair->remoteNode->status = FileNode::Status::MovedDirty;
                        if (pPair->localNode->status = FileNode::Status::MovedDirty)
                        {
                            pPair->SetDefaultAction(PairedNode::Action::FastForward);
                        }
                        else
                        {
                            pPair->SetDefaultAction(PairedNode::Action::Conflict);
                        }
                        break;
                    case HASHCMP_NE:
                    case HASHCMP_OTHERNULL:
                        pPair->remoteNode->status = FileNode::Status::MovedDirty;
                        pPair->SetDefaultAction(PairedNode::Action::Conflict);
                        break;
                    case HASHCMP_ONENULL:
                    case HASHCMP_EQPTR:
                    default:
                        break;
                    }
                }
            }
            else
            {
                pairedNodes.push_back(PairedNode(remoteNode.path, nullptr, nullptr, &remoteNode));
            }
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
}
