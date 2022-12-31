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

const std::map<PairedNode::Action, std::string> PairedNode::ActionAsString =
{
    {PairedNode::Action::None, "None"},
    {PairedNode::Action::DoNothing, "None"},
    {PairedNode::Action::LocalToRemote, "===>"},
    {PairedNode::Action::RemoteToLocal, "<==="},
    {PairedNode::Action::Ignore, "Ignore"},
    {PairedNode::Action::Conflict, "Conflict"},
    {PairedNode::Action::FastForward, "Fast Forward"},
};

void PairedNode::SetDefaultAction(Action action)
{
    this->action = action;
    this->defaultAction = action;
}

std::string PairedNode::GetStatusString() const
{
    return (localNode? FileNode::StatusAsString.at(localNode->status): FileNode::StatusAsString.at(FileNode::Status::Absent))
           + " <-> "
           + (remoteNode? FileNode::StatusAsString.at(remoteNode->status): FileNode::StatusAsString.at(FileNode::Status::Absent));
}

std::string PairedNode::GetActionString() const
{
    if (action != PairedNode::Action::None)
        return ActionAsString.at(action);
    else
        return "";
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
