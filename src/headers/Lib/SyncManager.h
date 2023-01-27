#ifndef SRC_LIB_SYNC_MANAGER_H
#define SRC_LIB_SYNC_MANAGER_H

#include "Domain/PairedNode.h"
#include "Lib/DBConnector.h"
#include "Lib/SSHConnector.h"
#include "Lib/SFTPConnector.h"

namespace SyncManager
{
    int Sync(PairedNode* pNode, std::string& remoteRoot, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp, HistoryFileNodeDBConnector& db);
}

#endif
