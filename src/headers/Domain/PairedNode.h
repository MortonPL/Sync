#pragma once
#include <string>
#include "domain/FileNode.h"
#include "Domain/HistoryFileNode.h"

class PairedNode
{
public:
    PairedNode();
    PairedNode(std::string path, FileNode* localNode=nullptr, HistoryFileNode* historyNode=nullptr, FileNode* remoteNode=nullptr);
    ~PairedNode();

    std::string path = "";
    FileNode* localNode = nullptr;
    HistoryFileNode* historyNode = nullptr;
    FileNode* remoteNode = nullptr;
};
