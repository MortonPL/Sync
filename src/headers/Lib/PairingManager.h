#pragma once
#include <list>
#include <forward_list>
#include "Domain/PairedNode.h"
#include "Lib/Creeper.h"
#include "Lib/Mapper.h"

namespace PairingManager
{
    void PairAll(std::forward_list<FileNode>& scanNodes, std::forward_list<HistoryFileNode>& historyNodes,
                      std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper, Mapper& mapper);

    void PairAllLocal(std::forward_list<FileNode>& scanNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper);
    void PairAllHistory(std::forward_list<HistoryFileNode>& historyNodes, std::list<PairedNode>& pairedNodes, Mapper& mapper);
    void PairAllRemote(std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper, Mapper& mapper);
}
