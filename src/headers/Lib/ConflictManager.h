#ifndef LIB_CONFLICT_MANAGER_H
#define LIB_CONFLICT_MANAGER_H

#include "Domain/PairedNode.h"
#include "Domain/ConflictRule.h"
#include "Lib/SFTPConnector.h"
#include "Lib/Announcer.h"

namespace ConflictManager
{
    extern std::string tempSuffixLocal;
    extern std::string tempSuffixRemote;

    bool Fetch(const PairedNode& node, const ConflictRule& rule, const std::string& remoteRoot, const std::string& tempPath, const SSHConnector& ssh, const SFTPConnector& sftp);
    bool Resolve(const PairedNode& node, const ConflictRule& rule, Announcer::announcerType announcer);
}

#endif
