#pragma once

#include "Domain/PairedNode.h"
#include "Lib/DBConnector.h"
#include "Lib/SSHConnector.h"

namespace SyncManager
{
    int Sync(PairedNode* pNode, SSHConnector& ssh, DBConnector& db);
}
