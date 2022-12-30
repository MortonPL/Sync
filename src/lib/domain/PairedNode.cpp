#include "domain/PairedNode.h"

PairedNode::PairedNode()
{
}

PairedNode::PairedNode(std::string path, FileNode* localNode, HistoryFileNode* historyNode, FileNode* remoteNode)
{
    this->path = path;
    this->localNode = localNode;
    this->historyNode = historyNode;
    this->remoteNode = remoteNode;
}

PairedNode::~PairedNode()
{
}
