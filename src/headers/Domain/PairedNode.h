#pragma once
#include <string>
#include "domain/FileNode.h"
#include "Domain/HistoryFileNode.h"

#define HASHCMP_ONENULL 0
#define HASHCMP_OTHERNULL 1
#define HASHCMP_NE 2
#define HASHCMP_EQ 3
class PairedNode
{
public:
    PairedNode();
    PairedNode(const std::string path, FileNode localNode=FileNode(), HistoryFileNode historyNode=HistoryFileNode(), FileNode remoteNode=FileNode());
    ~PairedNode();

    enum Action: char
    {
        None = 0,
        DoNothing,
        Ignore,
        LocalToRemote,
        RemoteToLocal,
        Conflict,
        Resolve,
        FastForward,
    };

    enum Progress: char
    {
        NoProgress = 0,
        Canceled,
        Failed,
        Success,
    };

    bool operator<(const PairedNode& other) const
    {
        return path < other.path;
    }

    void SetDefaultAction(Action action);
    std::pair<std::string, std::string> GetStatusString() const;
    std::string GetActionString() const;
    std::string GetProgressString() const;
    std::string GetDefaultActionString() const;
    static int CompareNodeHashes(const FileNode& one, const FileNode& other);
    
    static const std::map<Action, std::string> ActionAsString;
    static const std::map<Progress, std::string> ProgressAsString;

    const std::string path;
    std::string pathHash;
    bool deleted = false;
    Action action = Action::None;
    Action defaultAction = Action::None;
    Progress progress = Progress::NoProgress;
    FileNode localNode;
    HistoryFileNode historyNode;
    FileNode remoteNode;
};
