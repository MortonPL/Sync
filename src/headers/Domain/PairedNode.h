#pragma once
#include <string>
#include "domain/FileNode.h"
#include "Domain/HistoryFileNode.h"

#define HASHCMP_EQPTR 0
#define HASHCMP_ONENULL 1
#define HASHCMP_OTHERNULL 2
#define HASHCMP_EQ 3
#define HASHCMP_NE 4
class PairedNode
{
public:
    PairedNode();
    PairedNode(const std::string path, FileNode* localNode=nullptr, HistoryFileNode* historyNode=nullptr, FileNode* remoteNode=nullptr);
    ~PairedNode();

    enum Action: char
    {
        None = 0,
        Conflict,
        FastForward,
    };

    bool operator<(const PairedNode& other) const
    {
        return path < other.path;
    }

    void SetDefaultAction(Action action);
    std::string GetStatusString() const;
    std::string GetActionString() const;
    static int CompareNodeHashes(const FileNode* one, const FileNode* other);
    
    static const std::map<Action, std::string> ActionAsString;

    const std::string path;
    Action action = Action::None;
    Action defaultAction = Action::None;
    FileNode* localNode;
    HistoryFileNode* historyNode;
    FileNode* remoteNode;
};
