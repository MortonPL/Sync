#pragma once
#include <list>
#include <forward_list>
#include "Domain/PairedNode.h"
#include "Lib/Creeper.h"

namespace PairingManager
{
    void DoEverything(std::forward_list<FileNode>& scanNodes, std::forward_list<HistoryFileNode>& historyNodes,
                      std::forward_list<FileNode>& remoteNodes, std::list<PairedNode>& pairedNodes, Creeper& creeper);
}
