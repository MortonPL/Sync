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
        it++;
        index--;
    }
    return &*it;
}

class PairingTest: public ::testing::Test
{
protected:
    PairingTest()
    {
        scanNodes = 
        {
            MakeFileNode("A", 0, 0, 0x0, 0x0),
        };
        historyNodes = 
        {

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
        // local
        MakeFileNode("A", 0, 0, 0x0, 0x0, FileNode::Status::New),
        // remote
        MakeFileNode("A", 0, 0, 0x0, 0x0, FileNode::Status::New),
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("A", &expectedNodes[0], nullptr, &expectedNodes[1], PairedNode::Action::FastForward),
    };
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);

    // asserts
    auto pair = GetNodeAt(pairedNodes, 0);
    auto expected = GetNodeAt(expectedResult, 0);
    EXPECT_EQ(pair->path, expected->path);
    EXPECT_EQ(pair->localNode->status, expected->localNode->status);
    EXPECT_EQ(pair->remoteNode->status, expected->remoteNode->status);
}
