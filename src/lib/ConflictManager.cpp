#include "Lib/ConflictManager.h"

#include "Lib/FileSystem.h"
#include "Utils.h"

bool ConflictManager::Resolve(PairedNode* pNode, ConflictRule& rule, std::string& remoteRoot, SFTPConnector& sftp, Announcer::announcerType announcer)
{
    if (rule.command == "")
        return false;
    
    if (pNode->defaultAction != PairedNode::Action::Conflict)
        return false;
    
    if (pNode->historyNode.status == FileNode::Status::DeletedLocal || pNode->historyNode.status == FileNode::Status::DeletedRemote)
        return false;
    
    // get both files to tmp/
    std::string hashedPath = Utils::GetTempPath() + pNode->pathHash;
    std::string tempPathLocal = hashedPath + ".SyncLOCAL";
    std::string tempPathRemote = hashedPath + ".SyncREMOTE";

    if (!FileSystem::CopyLocalFile(pNode->path, tempPathLocal, std::filesystem::copy_options::skip_existing))
        return false;
    
    if (!sftp.ReceiveNonAtomic(tempPathRemote, remoteRoot + pNode->path))
        return false;

    // substitute command
    std::size_t pos;
    std::string result = rule.command;
    Utils::Replace(result, "$LOCAL", tempPathLocal);
    Utils::Replace(result, "$REMOTE", tempPathRemote);

    // launch it
    if (system(result.c_str()) != 0)
    {
        announcer("Conflict rule " + rule.name + " failed while executing the following command:\n" + result, SEV_ERROR);
        return false;
    }

    // check results

    // apply changes back
    pNode->SetDefaultAction(PairedNode::Action::Resolved);

    return true;
}
