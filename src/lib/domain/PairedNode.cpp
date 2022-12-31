#include "domain/PairedNode.h"

PairedNode::PairedNode()
{
}

PairedNode::PairedNode(const std::string path, FileNode* localNode, HistoryFileNode* historyNode, FileNode* remoteNode)
: path(path), localNode(localNode), historyNode(historyNode), remoteNode(remoteNode)
{
}

PairedNode::~PairedNode()
{
}

std::string PairedNode::GetStatusString() const
{
    return (localNode? FileNode::StatusAsString.at(localNode->status): FileNode::StatusAsString.at(FileNode::Status::Absent))
           + " <-> "
           + (remoteNode? FileNode::StatusAsString.at(remoteNode->status): FileNode::StatusAsString.at(FileNode::Status::Absent));
}

int PairedNode::CompareNodeHashes(const FileNode* one, const FileNode* other)
{
    if (one == other)
        return HASHCMP_EQPTR;
    if (!one && other)
        return HASHCMP_ONENULL;
    if (one && !other)
        return HASHCMP_OTHERNULL;
    return one->IsEqualHash(*other)? HASHCMP_EQ: HASHCMP_NE;
}
