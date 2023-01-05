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
    {PairedNode::Action::None, ""},
    {PairedNode::Action::DoNothing, ""},
    {PairedNode::Action::LocalToRemote, "==>>"},
    {PairedNode::Action::RemoteToLocal, "<<=="},
    {PairedNode::Action::Ignore, "Ignore"},
    {PairedNode::Action::Conflict, "Conflict"},
    {PairedNode::Action::Resolved, "=><="},
    {PairedNode::Action::FastForward, "Fast Forward"},
};

const std::map<PairedNode::Progress, std::string> PairedNode::ProgressAsString =
{
    {PairedNode::Progress::NoProgress, ""},
    {PairedNode::Progress::Canceled, "Canceled"},
    {PairedNode::Progress::Failed, "Failed"},
    {PairedNode::Progress::Success, "Success"},
};

void PairedNode::SetDefaultAction(Action action)
{
    this->action = action;
    this->defaultAction = action;
}

std::pair<std::string, std::string> PairedNode::GetStatusString() const
{
    return {FileNode::StatusAsString.at(localNode.status),
           FileNode::StatusAsString.at(remoteNode.status)};
}

std::string PairedNode::GetActionString() const
{
    return ActionAsString.at(action);
}

std::string PairedNode::GetProgressString() const
{
    return ProgressAsString.at(progress);
}

std::string PairedNode::GetDefaultActionString() const
{
    return ActionAsString.at(defaultAction);
}

int PairedNode::CompareNodeHashes(const FileNode& one, const FileNode& other)
{
    if (one.IsEmpty() && !other.IsEmpty())
        return HASHCMP_ONENULL;
    if (!one.IsEmpty() && other.IsEmpty())
        return HASHCMP_OTHERNULL;
    return one.IsEqualHash(other)? HASHCMP_EQ: HASHCMP_NE;
}
