#include "domain/PairedNode.h"

PairedNode::PairedNode()
{
}

PairedNode::PairedNode(const std::string path, FileNode localNode, HistoryFileNode historyNode, FileNode remoteNode)
: path(path)
{
    this->localNode = localNode;
    this->historyNode = historyNode;
    this->remoteNode = remoteNode;
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
    {PairedNode::Action::Cancel, "Cancel"},
};

void PairedNode::SetDefaultAction(Action action)
{
    this->action = action;
    this->defaultAction = action;
}

std::string PairedNode::GetStatusString() const
{
    return FileNode::StatusAsString.at(localNode.status)
           + " <-> "
           + FileNode::StatusAsString.at(remoteNode.status);
}

std::string PairedNode::GetActionString() const
{
    if (action != PairedNode::Action::None)
        return ActionAsString.at(action);
    else
        return "";
}

std::string PairedNode::GetDefaultActionString() const
{
    if (defaultAction != PairedNode::Action::None)
        return ActionAsString.at(defaultAction);
    else
        return "";
}

int PairedNode::CompareNodeHashes(const FileNode& one, const FileNode& other)
{
    if (one.IsEmpty() && !other.IsEmpty())
        return HASHCMP_ONENULL;
    if (!one.IsEmpty() && other.IsEmpty())
        return HASHCMP_OTHERNULL;
    return one.IsEqualHash(other)? HASHCMP_EQ: HASHCMP_NE;
}
