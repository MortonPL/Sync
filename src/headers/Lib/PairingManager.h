#ifndef SRC_LIB_PAIRING_MANAGER_H
#define SRC_LIB_PAIRING_MANAGER_H
#include <list>
#include <forward_list>
#include "Domain/PairedNode.h"
#include "Lib/Creeper.h"
#include "Lib/Mapper.h"

namespace PairingManager
{
    void PairAll(std::forward_list<FileNode>& scanNodes, std::forward_list<HistoryFileNode>& historyNodes,
                      std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper, Mapper& mapper);

    void PairAllLocal(const std::forward_list<FileNode>& scanNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper);
    void PairAllHistory(const std::forward_list<HistoryFileNode>& historyNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper);
    void PairAllRemote(std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper, Mapper& mapper);
    void SolveFinalAction(std::list<PairedNode>& pairedNodes);

    bool CheckChanges(FileNode& localOld, const FileNode& localNew, FileNode& remoteOld, const FileNode& remoteNew);
}

#endif
