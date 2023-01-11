#include "Lib/ConflictManager.h"

#include "Lib/FileSystem.h"
#include "Lib/Creeper.h"
#include "Lib/Compression.h"
#include "Utils.h"

std::string ConflictManager::tempSuffixLocal = ".SyncLOCAL";
std::string ConflictManager::tempSuffixRemote = ".SyncREMOTE";

bool ConflictManager::Fetch(PairedNode* pNode, ConflictRule& rule, std::string& remoteRoot, std::string& tempPath, SSHConnector& ssh, SFTPConnector& sftp)
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
    std::string remoteTempPath = tempPath + pNode->pathHash;

    if (!FileSystem::CopyLocalFile(pNode->path, tempPathLocal, std::filesystem::copy_options::skip_existing))
        return false;
    
    // receive the remote file
    // compress if > 50MB
    if (pNode->remoteNode.size > 50000000)
    {
        // if we already have the uncompressed file transfered, don't bother, just move
        struct stat buf;
        if (stat(hashedPath.c_str(), &buf) != 0 || buf.st_size != pNode->remoteNode.size)
        {
            off_t compressedSize = 0;
            if (ssh.CallCLICompress(remoteRoot + pNode->path, remoteTempPath+".zst", &compressedSize) != CALLCLI_OK)
                return false;
            if (!sftp.ReceiveNonAtomic(remoteTempPath+".zst", hashedPath+".zst"))
                return false;
            if (!Compression::Decompress(hashedPath+".zst", hashedPath))
                return false;
        }
        if (!FileSystem::MoveLocalFile(hashedPath, tempPathRemote))
            return false;
        sftp.Delete(remoteTempPath+".zst");
        remove((hashedPath+".zst").c_str());
    }
    else
    {
        if (!sftp.ReceiveNonAtomic(remoteRoot + pNode->path, tempPathRemote))
            return false;
    }
    return true;
}

bool ConflictManager::Resolve(PairedNode* pNode, ConflictRule& rule, Announcer::announcerType announcer)
{
    std::string hashedPath = Utils::GetTempPath() + pNode->pathHash;
    std::string tempPathLocal = hashedPath + tempSuffixLocal;
    std::string tempPathRemote = hashedPath + tempSuffixRemote;

    // substitute command
    std::string result = rule.command;
    Utils::Replace(result, "$LOCAL", tempPathLocal);
    Utils::Replace(result, "$REMOTE", tempPathRemote);

    // launch it
    if (system(result.c_str()) != 0)
    {
        announcer("Conflict rule " + rule.name + " failed while executing the following command:\n" + result, Announcer::Severity::Error);
        return false;
    }

    // check results
    if (!std::filesystem::exists(tempPathLocal) || !std::filesystem::is_regular_file(tempPathLocal))
    {
        announcer("The command was executed, but the expected temporary file:\n" + tempPathLocal + "\ndoesn't exist!" , Announcer::Severity::Error);
        return false;
    }

    if (!std::filesystem::exists(tempPathRemote) || !std::filesystem::is_regular_file(tempPathLocal))
    {
        announcer("The command was executed, but the expected temporary file:\n" + tempPathRemote + "\ndoesn't exist!" , Announcer::Severity::Error);
        return false;
    }

    FileNode local;
    FileNode remote;
    Creeper::MakeSingleNode(tempPathLocal, local);
    Creeper::MakeSingleNode(tempPathRemote, remote);
    if (!local.IsEqualHash(remote))
    {
        announcer("The command was executed, but temporary files:\n" + tempPathLocal + "\nand\n" + tempPathRemote + "\nhave different content!" , Announcer::Severity::Error);
        return false;
    }

    // apply changes back
    pNode->SetDefaultAction(PairedNode::Action::Resolve);

    return true;
}
