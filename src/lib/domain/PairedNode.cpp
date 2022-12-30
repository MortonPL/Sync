#include "domain/PairedNode.h"

PairedNode::PairedNode()
{
}

PairedNode::PairedNode(const std::string path, const FileNode* localNode, const HistoryFileNode* historyNode, const FileNode* remoteNode)
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
