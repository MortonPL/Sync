#ifndef SRC_DOMAIN_PAIRED_NODE_H
#define SRC_DOMAIN_PAIRED_NODE_H
#include <string>
#include "Domain/FileNode.h"
#include "Domain/HistoryFileNode.h"
class PairedNode
{
public:
    PairedNode();
    PairedNode(const std::string path, const FileNode localNode=FileNode(), const HistoryFileNode historyNode=HistoryFileNode(), const FileNode remoteNode=FileNode());
    ~PairedNode();

    enum class Action: char
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

    enum class Progress: char
    {
        NoProgress = 0,
        Canceled,
        Failed,
        Success,
    };

    enum class HashComparisonResult: char
    {
        OneNull = 0,
        OtherNull,
        Equal,
        NotEqual,
    };

    bool operator<(const PairedNode& other) const
    {
        return path < other.path;
    }

    void SetDefaultAction(const Action action);
    std::pair<std::string, std::string> GetStatusString() const;
    std::string GetActionString() const;
    std::string GetProgressString() const;
    std::string GetDefaultActionString() const;
    static HashComparisonResult CompareNodeHashes(const FileNode& one, const FileNode& other);
    
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
    std::string failureReason;
};

#endif
