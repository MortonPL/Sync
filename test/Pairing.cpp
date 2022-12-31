#include <gtest/gtest.h>

#include <list>
#include <forward_list>
#include "Lib/PairingManager.h"

FileNode MakeFileNode(std::string path, dev_t dev, ino_t inode, XXH64_hash_t hashHigh, XXH64_hash_t hashLow, FileNode::Status status=FileNode::Status::New)
{
    FileNode f(path, dev, inode, 0, 0, hashHigh, hashLow);
    f.status = status;
    return f;
}

HistoryFileNode MakeHistoryFileNode(std::string path, dev_t dev, ino_t inode, dev_t rdev, ino_t rinode, XXH64_hash_t hashHigh, XXH64_hash_t hashLow, FileNode::Status status=FileNode::Status::HistoryPresent)
{
    HistoryFileNode h(path, dev, inode, rdev, rinode, 0, 0, hashHigh, hashLow);
    h.status = status;
    return h;
}

PairedNode MakePairedNode(std::string path, FileNode* fileNode, HistoryFileNode* historyNode, FileNode* remoteNode, PairedNode::Action action=PairedNode::Action::None)
{
    PairedNode p(path, fileNode, historyNode, remoteNode);
    p.action = action;
    return p;
}

template < typename T, template <class ...> class C>
T* GetNodeAt(C<T>& list, int index)
{
    auto it = list.begin();
    while (index > 0)
    {
        if (it == list.end())
            return nullptr;
        it++;
        index--;
    }
    return &*it;
}

void ExpectPairedNode(PairedNode* pair, PairedNode* expected)
{
    EXPECT_NE(pair, nullptr) << "PairedNode does not exist!";
    EXPECT_EQ(pair->path, expected->path) << "PairedNode " << expected->path << " has wrong path!";
    EXPECT_EQ(pair->action, expected->action) << "PairedNode " << expected->path << " has wrong action!";
    if (expected->localNode)
        EXPECT_EQ(pair->localNode->status, expected->localNode->status) << "PairedNode " << expected->path << " has local status mismatch!";
    else
        EXPECT_EQ(pair->localNode, nullptr);
    if (expected->historyNode)
        EXPECT_EQ(pair->historyNode->status, expected->historyNode->status) << "PairedNode " << expected->path << " has history status mismatch!";
    else
        EXPECT_EQ(pair->historyNode, nullptr);
    if (expected->remoteNode)
        EXPECT_EQ(pair->remoteNode->status, expected->remoteNode->status) << "PairedNode " << expected->path << " has remote status mismatch!";
    else
        EXPECT_EQ(pair->remoteNode, nullptr);
}

/*
LOCAL
  A(0x00)
  B(0x00)
  C(0x00)
HISTORY
  A(0x00)
  B(0x00)
REMOTE
  A(0x00)
*/
class PairingTest: public ::testing::Test
{
protected:
    PairingTest()
    {
        scanNodes = 
        {
            MakeFileNode("A", 0, 0, 0x0, 0x0),
            MakeFileNode("B", 0, 1, 0x0, 0x0),
            MakeFileNode("C", 0, 2, 0x0, 0x0),
        };
        historyNodes = 
        {
            MakeHistoryFileNode("A", 0, 0, 0, 0, 0x0, 0x0),
            MakeHistoryFileNode("B", 0, 1, 0, 1, 0x0, 0x0),
        };
        remoteNodes = 
        {
            MakeFileNode("A", 0, 0, 0x0, 0x0),
        };
    };

    std::forward_list<FileNode> scanNodes;
    std::forward_list<HistoryFileNode> historyNodes;
    std::forward_list<FileNode> remoteNodes;
    std::list<PairedNode> pairedNodes;
    Creeper emptyCreeper;
    Mapper mapper;
};

TEST_F(PairingTest, Simple)
{
    // OUT
    std::vector<FileNode> expectedNodes = 
    {
        MakeFileNode("A", 0, 0, 0x0, 0x0, FileNode::Status::Clean), // L
        MakeFileNode("A", 0, 0, 0x0, 0x0, FileNode::Status::Clean), // R
        MakeFileNode("B", 0, 1, 0x0, 0x0, FileNode::Status::Clean), // L
        MakeFileNode("C", 0, 2, 0x0, 0x0, FileNode::Status::New), // L
    };
    std::vector<HistoryFileNode> expectedHistory = 
    {
        MakeHistoryFileNode("A", 0, 0, 0, 0, 0x0, 0x0, FileNode::Status::HistoryPresent),
        MakeHistoryFileNode("B", 0, 1, 0, 1, 0x0, 0x0, FileNode::Status::DeletedRemote),
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("A", &expectedNodes[0], &expectedHistory[0], &expectedNodes[1], PairedNode::Action::DoNothing),
        MakePairedNode("B", &expectedNodes[2], &expectedHistory[1], nullptr, PairedNode::Action::RemoteToLocal),
        MakePairedNode("C", &expectedNodes[3], nullptr, nullptr, PairedNode::Action::LocalToRemote),
    };
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);

    // asserts
    ExpectPairedNode(GetNodeAt(pairedNodes, 0), GetNodeAt(expectedResult, 0));
    ExpectPairedNode(GetNodeAt(pairedNodes, 1), GetNodeAt(expectedResult, 1));
    ExpectPairedNode(GetNodeAt(pairedNodes, 2), GetNodeAt(expectedResult, 2));
}
