#include "Lib/ConflictManager.h"

#include "Lib/FileSystem.h"
#include "Lib/Creeper.h"
#include "Utils.h"

std::string ConflictManager::tempSuffixLocal = ".SyncLOCAL";
std::string ConflictManager::tempSuffixRemote = ".SyncREMOTE";

bool ConflictManager::Resolve(PairedNode* pNode, ConflictRule& rule, std::string& remoteRoot, SFTPConnector& sftp, Announcer::announcerType announcer)
{
    if (rule.command == "")
        return false;
    
    if (pNode->defaultAction != PairedNode::Action::Conflict)
        return false;
    
    if (pNode->localNode.IsEmpty() || pNode->remoteNode.IsEmpty())
        return false;
    
    // get both files to tmp/
    std::string hashedPath = Utils::GetTempPath() + pNode->pathHash;
    std::string tempPathLocal = hashedPath + tempSuffixLocal;
    std::string tempPathRemote = hashedPath + tempSuffixRemote;

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
    if (!std::filesystem::exists(tempPathLocal) || !std::filesystem::is_regular_file(tempPathLocal))
    {
        announcer("The command was executed, but the expected temporary file:\n" + tempPathLocal + "\ndoesn't exist!" , SEV_ERROR);
        return false;
    }

    if (!std::filesystem::exists(tempPathRemote) || !std::filesystem::is_regular_file(tempPathLocal))
    {
        announcer("The command was executed, but the expected temporary file:\n" + tempPathRemote + "\ndoesn't exist!" , SEV_ERROR);
        return false;
    }

    FileNode local;
    FileNode remote;
    Creeper::MakeSingleNode(tempPathLocal, local);
    Creeper::MakeSingleNode(tempPathRemote, remote);
    if (!local.IsEqualHash(remote))
    {
        announcer("The command was executed, but temporary files:\n" + tempPathLocal + "\nand\n" + tempPathRemote + "\nhave different content!" , SEV_ERROR);
        return false;
    }

    // apply changes back
    pNode->SetDefaultAction(PairedNode::Action::Resolve);

    return true;
}
