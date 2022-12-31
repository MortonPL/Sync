#include "Lib/PairingManager.h"

#include "Lib/Mapper.h"

void PairLocal(std::forward_list<FileNode>& scanNodes, std::list<PairedNode>& pairedNodes)
{
    for (auto& scanNode: scanNodes)
    {
        pairedNodes.push_back(PairedNode(scanNode.path, &scanNode));
        Mapper::EmplaceMapPath(scanNode.path, pairedNodes.back());
        Mapper::EmplaceMapLocalInode(scanNode.GetDevInode(), pairedNodes.back());
    }
}

void PairHistory(std::forward_list<HistoryFileNode>& historyNodes, std::list<PairedNode>& pairedNodes)
{
    for (auto& historyNode: historyNodes)
    {
        auto pPair = Mapper::FindMapPath(historyNode.path);
        if (pPair)
        {
            pPair->historyNode = &historyNode;
            Mapper::EmplaceMapLocalInode(historyNode.GetDevInode(), *pPair);
            Mapper::EmplaceMapRemoteInode(historyNode.GetRemoteDevInode(), *pPair);
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
            pPair = Mapper::FindMapLocalInode(historyNode.GetDevInode());
            if (pPair)
            {
                pPair->historyNode = &historyNode;
                Mapper::EmplaceMapPath(historyNode.path, *pPair);
                Mapper::EmplaceMapRemoteInode(historyNode.GetRemoteDevInode(), *pPair);
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
                Mapper::EmplaceMapPath(historyNode.path, *pPair);
                Mapper::EmplaceMapRemoteInode(historyNode.GetRemoteDevInode(), *pPair);
                pPair->historyNode->status = FileNode::Status::Deleted;
            }
        }
    }
}

void PairRemote(std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper)
{
for(auto& remoteNode: remoteNodes)
    {
        if (creeper.CheckIfFileIsIgnored(remoteNode.path))
            continue;
        auto pPair = Mapper::FindMapPath(remoteNode.path);
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
            pPair = Mapper::FindMapRemoteInode(remoteNode.GetDevInode());
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

void PairingManager::DoEverything(std::forward_list<FileNode>& scanNodes, std::forward_list<HistoryFileNode>& historyNodes,
                                  std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper)
{
    //pair local
    PairLocal(scanNodes, pairedNodes);

    //pair history
    PairHistory(historyNodes, pairedNodes);

    //pair remote
    PairRemote(remoteNodes, pairedNodes, creeper);
}
