#pragma once

#include "Domain/PairedNode.h"
#include "Domain/ConflictRule.h"
#include "Lib/SFTPConnector.h"

namespace ConflictManager
{
    void Resolve(PairedNode* pNode, ConflictRule& rule, SFTPConnector& sftp);
};
