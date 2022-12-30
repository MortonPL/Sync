#pragma once
#include <string>
#include "domain/FileNode.h"
#include "Domain/HistoryFileNode.h"

class PairedNode
{
public:
    PairedNode();
    PairedNode(const std::string path, const FileNode* localNode=nullptr, const HistoryFileNode* historyNode=nullptr, const FileNode* remoteNode=nullptr);
    ~PairedNode();

    bool operator<(const PairedNode& other) const
    {
        return path < other.path;
    }

    std::string GetStatusString() const;

    const std::string path;
    const FileNode* localNode;
    const HistoryFileNode* historyNode;
    const FileNode* remoteNode;
};
