#pragma once

#include "Domain/PairedNode.h"
#include "Domain/ConflictRule.h"
#include "Lib/SFTPConnector.h"
#include "Lib/Announcer.h"

namespace ConflictManager
{
    bool Resolve(PairedNode* pNode, ConflictRule& rule, std::string& remoteRoot, SFTPConnector& sftp, Announcer::announcerType announcer);
};
