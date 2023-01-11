#include "Lib/ConflictManager.h"

#include "Lib/FileSystem.h"
#include "Lib/Creeper.h"
#include "Lib/Compression.h"
#include "Utils.h"

std::string ConflictManager::tempSuffixLocal = ".SyncLOCAL";
std::string ConflictManager::tempSuffixRemote = ".SyncREMOTE";

bool ConflictManager::Fetch(const PairedNode& node, const ConflictRule& rule, const std::string& remoteRoot, const std::string& tempPath, const SSHConnector& ssh, const SFTPConnector& sftp)
{
    const std::string compressedExtension = ".zst";

    if (rule.command == "")
        return false;
    
    if (node.defaultAction != PairedNode::Action::Conflict)
        return false;
    
    if (node.localNode.IsEmpty() || node.remoteNode.IsEmpty())
        return false;
    
    // get both files to tmp/
    const std::string hashedPath = Utils::GetTempPath() + node.pathHash;
    const std::string tempFileLocal = hashedPath + tempSuffixLocal;
    const std::string tempFileRemote = hashedPath + tempSuffixRemote;
    const std::string remoteTempPath = tempPath + node.pathHash;

    if (!FileSystem::CopyLocalFile(node.path, tempFileLocal, std::filesystem::copy_options::skip_existing))
        return false;
    
    // receive the remote file
    // compress if > 50MB
    if (node.remoteNode.size >= Compression::minimumCompressibleSize)
    {
        // only transfer if we don't have the matching uncompressed file
        struct stat buf;
        if (stat(hashedPath.c_str(), &buf) != 0 || buf.st_size != node.remoteNode.size)
        {
            off_t compressedSize = 0;
            if (ssh.CallCLICompress(remoteRoot + node.path, remoteTempPath + compressedExtension, &compressedSize) != CALLCLI_OK)
                return false;
            if (!sftp.ReceiveNonAtomic(remoteTempPath + compressedExtension, hashedPath + compressedExtension))
                return false;
            if (!Compression::Decompress(hashedPath + compressedExtension, hashedPath))
                return false;
        }
        if (!FileSystem::MoveLocalFile(hashedPath, tempFileRemote))
            return false;
        sftp.Delete(remoteTempPath + compressedExtension);
        std::filesystem::remove(hashedPath + compressedExtension);
    }
    else
    {
        if (!sftp.ReceiveNonAtomic(remoteRoot + node.path, tempFileRemote))
            return false;
    }
    return true;
}

bool ConflictManager::Resolve(const PairedNode& node, const ConflictRule& rule, Announcer::announcerType announcer)
{
    const std::string hashedPath = Utils::GetTempPath() + node.pathHash;
    const std::string tempFileLocal = hashedPath + tempSuffixLocal;
    const std::string tempFileRemote = hashedPath + tempSuffixRemote;

    // substitute command
    std::string result = rule.command;
    Utils::Replace(result, "$LOCAL", tempFileLocal);
    Utils::Replace(result, "$REMOTE", tempFileRemote);

    // launch it
    if (system(result.c_str()) != 0)
    {
        announcer("Conflict rule " + rule.name + " failed while executing the following command:\n" + result, Announcer::Severity::Error);
        return false;
    }

    // check results
    auto resultCheck = [announcer](const std::string& path)
    {
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
        {
            announcer("The command was executed, but the expected temporary file:\n" + path + "\ndoesn't exist!" , Announcer::Severity::Error);
            return false;
        }
        return true;
    };
    if (!resultCheck(tempFileLocal))
        return false;
    if (!resultCheck(tempFileRemote))
        return false;

    FileNode local;
    FileNode remote;
    Creeper::MakeSingleNode(tempFileLocal, local);
    Creeper::MakeSingleNode(tempFileRemote, remote);
    if (!local.IsEqualHash(remote))
    {
        announcer("The command was executed, but temporary files:\n" + tempFileLocal + "\nand\n" + tempFileRemote + "\nhave different content!" , Announcer::Severity::Error);
        return false;
    }

    return true;
}
