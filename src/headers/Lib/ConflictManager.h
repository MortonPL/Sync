#pragma once

#include "Domain/PairedNode.h"
#include "Domain/ConflictRule.h"
#include "Lib/SFTPConnector.h"
#include "Lib/Announcer.h"

namespace ConflictManager
{
    extern std::string tempSuffixLocal;
    extern std::string tempSuffixRemote;

    bool Fetch(PairedNode* pNode, ConflictRule& rule, std::string& remoteRoot, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp);
    bool Resolve(PairedNode* pNode, ConflictRule& rule, Announcer::announcerType announcer);
}
