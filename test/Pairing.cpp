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
    p.defaultAction = action;
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
    EXPECT_NE(expected, nullptr) << "Too many PairedNodes!";
    EXPECT_EQ(pair->path, expected->path) << "PairedNode " << expected->path << " has wrong path!";
    EXPECT_EQ(pair->action, expected->action) << "PairedNode " << expected->path << " has wrong action!";
    EXPECT_EQ(pair->defaultAction, expected->defaultAction) << "PairedNode " << expected->path << " has wrong default action!";
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
COMMONLY USED NAMES:
    eq - equal (clean)
    neq - not equal (dirty)
    meq - moved equal (moved clean)
    mneq - moved not equal (moved dirty)
    old - saved in history
*/
class PairingBasicTest: public ::testing::Test
{
protected:
    PairingBasicTest()
    {
    };

    std::list<PairedNode> pairedNodes;
    Creeper emptyCreeper;
    Mapper mapper;
};

#define EXPECT_ALL_NODES(pairedNodes, expectedResult)\
{\
    auto pit = pairedNodes.begin();\
    auto eit = expectedResult.begin();\
    for (int i = 0; i < expectedResult.size(); i++)\
    {\
        ExpectPairedNode(pit != pairedNodes.end()? &*pit: nullptr, eit != expectedResult.end()? &*eit: nullptr);\
        pit++;\
        eit++;\
    }\
}

/*
LOCAL               HISTORY             REMOTE
  noh/                noh/                noh/
    eq (0x00)           -                   eq (0x00)
    neq(0x00)           -                   neq(0x01)
    loc(0x00)           -                   -
    -                   -                   rem(0x00)
*/
TEST_F(PairingBasicTest, NoHistory)
{
    // IN
    std::forward_list<FileNode> scanNodes =
    {
        MakeFileNode("noh/eq", 0, 0, 0x0, 0x0),
        MakeFileNode("noh/neq", 0, 1, 0x0, 0x0),
        MakeFileNode("noh/loc", 0, 2, 0x0, 0x0),
    };
    std::forward_list<HistoryFileNode> historyNodes = {};
    std::forward_list<FileNode> remoteNodes =
    {
        MakeFileNode("noh/eq", 0, 0, 0x0, 0x0),
        MakeFileNode("noh/neq", 0, 1, 0x0, 0x1),
        MakeFileNode("noh/rem", 0, 2, 0x0, 0x0),
    };

    // OUT
    std::vector<FileNode> expectedNodes = 
    {
        MakeFileNode("noh/eq", 0, 0, 0x0, 0x0, FileNode::New), // L
        MakeFileNode("noh/eq", 0, 0, 0x0, 0x0, FileNode::New), // R
        MakeFileNode("noh/neq", 0, 1, 0x0, 0x0, FileNode::New), // L
        MakeFileNode("noh/neq", 0, 1, 0x1, 0x0, FileNode::New), // R
        MakeFileNode("noh/loc", 0, 2, 0x0, 0x0, FileNode::New), // L
        MakeFileNode("noh/rem", 0, 2, 0x0, 0x0, FileNode::New),  // R
    };
    std::vector<HistoryFileNode> expectedHistory = 
    {
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("noh/eq", &expectedNodes[0], nullptr, &expectedNodes[1], PairedNode::FastForward),
        MakePairedNode("noh/neq", &expectedNodes[2], nullptr, &expectedNodes[3], PairedNode::Conflict),
        MakePairedNode("noh/loc", &expectedNodes[4], nullptr, nullptr, PairedNode::LocalToRemote),
        MakePairedNode("noh/rem", nullptr, nullptr, &expectedNodes[5], PairedNode::RemoteToLocal),
    };

    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

/*
LOCAL               HISTORY             REMOTE
  ltor/               ltor/               ltor/
    neq (0x01)          neq (0x00)          neq (0x00)
    meq (0x00)          old1(0x00)          old1(0x00)
    mneq(0x01)          old2(0x00)          old2(0x00)
    -                   old3(0x00)          old3(0x00)
*/
TEST_F(PairingBasicTest, LocalToRemote)
{
    // IN
    std::forward_list<FileNode> scanNodes =
    {
        MakeFileNode("ltor/neq", 0, 0, 0x0, 0x1),
        MakeFileNode("ltor/meq", 0, 1, 0x0, 0x0),
        MakeFileNode("ltor/mneq", 0, 2, 0x0, 0x1),
        // ltor/old3
    };
    std::forward_list<HistoryFileNode> historyNodes =
    {
        MakeHistoryFileNode("ltor/neq", 0, 0, 0, 0, 0x0, 0x0),
        MakeHistoryFileNode("ltor/old1", 0, 1, 0, 1, 0x0, 0x0),
        MakeHistoryFileNode("ltor/old2", 0, 2, 0, 2, 0x0, 0x0),
        MakeHistoryFileNode("ltor/old3", 0, 3, 0, 3, 0x0, 0x0),
    };
    std::forward_list<FileNode> remoteNodes =
    {
        MakeFileNode("ltor/neq", 0, 0, 0x0, 0x0),
        MakeFileNode("ltor/old1", 0, 1, 0x0, 0x0),
        MakeFileNode("ltor/old2", 0, 2, 0x0, 0x0),
        MakeFileNode("ltor/old3", 0, 3, 0x0, 0x0),
    };

    // OUT
    std::vector<FileNode> expectedNodes = 
    {
        MakeFileNode("ltor/neq", 0, 0, 0x0, 0x1, FileNode::Dirty), // L
        MakeFileNode("ltor/neq", 0, 0, 0x0, 0x0, FileNode::Clean), // R
        MakeFileNode("ltor/meq", 0, 1, 0x0, 0x0, FileNode::MovedClean), // L
        MakeFileNode("ltor/old1", 0, 1, 0x0, 0x0, FileNode::Clean), // R
        MakeFileNode("ltor/mneq", 0, 2, 0x0, 0x1, FileNode::MovedDirty), // L
        MakeFileNode("ltor/old2", 0, 2, 0x0, 0x0, FileNode::Clean), // R
        MakeFileNode("ltor/old3", 0, 3, 0x0, 0x0, FileNode::Clean), // R
    };
    std::vector<HistoryFileNode> expectedHistory = 
    {
        MakeHistoryFileNode("ltor/neq", 0, 0, 0, 0, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("ltor/old1", 0, 1, 0, 1, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("ltor/old2", 0, 2, 0, 2, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("ltor/old3", 0, 3, 0, 3, 0x0, 0x0, FileNode::DeletedLocal),
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("ltor/neq", &expectedNodes[0], &expectedHistory[0], &expectedNodes[1], PairedNode::LocalToRemote),
        MakePairedNode("ltor/meq", &expectedNodes[2], &expectedHistory[1], &expectedNodes[3], PairedNode::LocalToRemote),
        MakePairedNode("ltor/mneq", &expectedNodes[4], &expectedHistory[2], &expectedNodes[5], PairedNode::LocalToRemote),
        MakePairedNode("ltor/old3", nullptr, &expectedHistory[3], &expectedNodes[6], PairedNode::LocalToRemote),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

/*
LOCAL               HISTORY             REMOTE
  rtol/               rtol/               rtol/
    neq (0x00)          neq (0x00)          neq (0x01)
    old1(0x00)          old1(0x00)          meq (0x00)
    old2(0x00)          old2(0x00)          mneq(0x01)
    old3(0x00)          old3(0x00)          -
*/
TEST_F(PairingBasicTest, RemoteToLocal)
{
    // IN
    std::forward_list<FileNode> scanNodes =
    {
        MakeFileNode("rtol/neq", 0, 0, 0x0, 0x0),
        MakeFileNode("rtol/old1", 0, 1, 0x0, 0x0),
        MakeFileNode("rtol/old2", 0, 2, 0x0, 0x0),
        MakeFileNode("rtol/old3", 0, 3, 0x0, 0x0),
    };
    std::forward_list<HistoryFileNode> historyNodes =
    {
        MakeHistoryFileNode("rtol/neq", 0, 0, 0, 0, 0x0, 0x0),
        MakeHistoryFileNode("rtol/old1", 0, 1, 0, 1, 0x0, 0x0),
        MakeHistoryFileNode("rtol/old2", 0, 2, 0, 2, 0x0, 0x0),
        MakeHistoryFileNode("rtol/old3", 0, 3, 0, 3, 0x0, 0x0),
    };
    std::forward_list<FileNode> remoteNodes =
    {
        MakeFileNode("rtol/neq", 0, 0, 0x0, 0x1),
        MakeFileNode("rtol/meq", 0, 1, 0x0, 0x0),
        MakeFileNode("rtol/mneq", 0, 2, 0x0, 0x1),
        // rtol/old3
    };

    // OUT
    std::vector<FileNode> expectedNodes = 
    {
        MakeFileNode("rtol/neq", 0, 0, 0x0, 0x0, FileNode::Clean), // L
        MakeFileNode("rtol/neq", 0, 0, 0x0, 0x1, FileNode::Dirty), // R
        MakeFileNode("rtol/old1", 0, 1, 0x0, 0x0, FileNode::Clean), // L
        MakeFileNode("rtol/meq", 0, 1, 0x0, 0x0, FileNode::MovedClean), // R
        MakeFileNode("rtol/old2", 0, 2, 0x0, 0x0, FileNode::Clean), // L
        MakeFileNode("rtol/mneq", 0, 2, 0x0, 0x1, FileNode::MovedDirty), // R
        MakeFileNode("rtol/old3", 0, 3, 0x0, 0x0, FileNode::Clean), // L
    };
    std::vector<HistoryFileNode> expectedHistory = 
    {
        MakeHistoryFileNode("rtol/neq", 0, 0, 0, 0, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("rtol/old1", 0, 1, 0, 1, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("rtol/old2", 0, 2, 0, 2, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("rtol/old3", 0, 3, 0, 3, 0x0, 0x0, FileNode::DeletedRemote),
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("rtol/neq", &expectedNodes[0], &expectedHistory[0], &expectedNodes[1], PairedNode::RemoteToLocal),
        MakePairedNode("rtol/old1", &expectedNodes[2], &expectedHistory[1], &expectedNodes[3], PairedNode::RemoteToLocal),
        MakePairedNode("rtol/old2", &expectedNodes[4], &expectedHistory[2], &expectedNodes[5], PairedNode::RemoteToLocal),
        MakePairedNode("rtol/old3", &expectedNodes[6], &expectedHistory[3], nullptr, PairedNode::RemoteToLocal),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

/*
LOCAL               HISTORY             REMOTE
  ffwd/               ffwd/               ffwd/
    eq  (0x00)          eq  (0x00)          eq  (0x00)
    neq (0x01)          neq (0x00)          neq (0x01)
    meq (0x00)          old1(0x00)          meq (0x00)
    mneq(0x01)          old2(0x00)          mneq(0x01)
    -                   old3(0x00)          -
*/
TEST_F(PairingBasicTest, FastForwards)
{
    // IN
    std::forward_list<FileNode> scanNodes =
    {
        MakeFileNode("ffwd/eq", 0, 0, 0x0, 0x0),
        MakeFileNode("ffwd/neq", 0, 1, 0x0, 0x1),
        MakeFileNode("ffwd/meq", 0, 2, 0x0, 0x0),
        MakeFileNode("ffwd/mneq", 0, 3, 0x0, 0x1),
        // ffwd/old3
    };
    std::forward_list<HistoryFileNode> historyNodes =
    {
        MakeHistoryFileNode("ffwd/eq", 0, 0, 0, 0, 0x0, 0x0),
        MakeHistoryFileNode("ffwd/neq", 0, 1, 0, 1, 0x0, 0x0),
        MakeHistoryFileNode("ffwd/old1", 0, 2, 0, 2, 0x0, 0x0),
        MakeHistoryFileNode("ffwd/old2", 0, 3, 0, 3, 0x0, 0x0),
        MakeHistoryFileNode("ffwd/old3", 0, 4, 0, 4, 0x0, 0x0),
    };
    std::forward_list<FileNode> remoteNodes =
    {
        MakeFileNode("ffwd/eq", 0, 0, 0x0, 0x0),
        MakeFileNode("ffwd/neq", 0, 1, 0x0, 0x1),
        MakeFileNode("ffwd/meq", 0, 2, 0x0, 0x0),
        MakeFileNode("ffwd/mneq", 0, 3, 0x0, 0x1),
        // ffwd/old3
    };

    // OUT
    std::vector<FileNode> expectedNodes = 
    {
        MakeFileNode("ffwd/eq", 0, 0, 0x0, 0x0, FileNode::Clean), // L
        MakeFileNode("ffwd/eq", 0, 0, 0x0, 0x0, FileNode::Clean), // R
        MakeFileNode("ffwd/neq", 0, 1, 0x0, 0x1, FileNode::Dirty), // L
        MakeFileNode("ffwd/neq", 0, 1, 0x0, 0x1, FileNode::Dirty), // R
        MakeFileNode("ffwd/meq", 0, 2, 0x0, 0x0, FileNode::MovedClean), // L
        MakeFileNode("ffwd/meq", 0, 2, 0x0, 0x0, FileNode::MovedClean), // R
        MakeFileNode("ffwd/mneq", 0, 3, 0x0, 0x1, FileNode::MovedDirty), // L
        MakeFileNode("ffwd/mneq", 0, 3, 0x0, 0x1, FileNode::MovedDirty), // L
    };
    std::vector<HistoryFileNode> expectedHistory = 
    {
        MakeHistoryFileNode("ffwd/eq", 0, 0, 0, 0, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("ffwd/neq", 0, 1, 0, 1, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("ffwd/old1", 0, 2, 0, 2, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("ffwd/old2", 0, 3, 0, 3, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("ffwd/old3", 0, 4, 0, 4, 0x0, 0x0, FileNode::DeletedBoth),
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("ffwd/eq", &expectedNodes[0], &expectedHistory[0], &expectedNodes[1], PairedNode::DoNothing),
        MakePairedNode("ffwd/neq", &expectedNodes[2], &expectedHistory[1], &expectedNodes[3], PairedNode::FastForward),
        MakePairedNode("ffwd/meq", &expectedNodes[4], &expectedHistory[2], &expectedNodes[5], PairedNode::FastForward),
        MakePairedNode("ffwd/mneq", &expectedNodes[6], &expectedHistory[3], &expectedNodes[7], PairedNode::FastForward),
        MakePairedNode("ffwd/old3", nullptr, &expectedHistory[4], nullptr, PairedNode::FastForward),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

/*
LOCAL               HISTORY             REMOTE
  del1/               del1/               del1/
    -                   neq (0x00)          neq (0x01)
    -                   old1(0x00)          meq (0x00)
    -                   old2(0x00)          mneq(0x01)
  del2/               del2/               del2/
    neq (0x01)          neq (0x00)          -
    meq (0x00)          old1(0x00)          -
    mneq(0x01)          old2(0x00)          -
*/
TEST_F(PairingBasicTest, Deletions)
{
    // IN
    std::forward_list<FileNode> scanNodes =
    {
        // del1/neq
        // del1/old1
        // del1/old2
        MakeFileNode("del2/neq", 0, 3, 0x0, 0x1),
        MakeFileNode("del2/meq", 0, 4, 0x0, 0x0),
        MakeFileNode("del2/mneq", 0, 5, 0x0, 0x1),
    };
    std::forward_list<HistoryFileNode> historyNodes =
    {
        MakeHistoryFileNode("del1/neq", 0, 0, 0, 0, 0x0, 0x0),
        MakeHistoryFileNode("del1/old1", 0, 1, 0, 1, 0x0, 0x0),
        MakeHistoryFileNode("del1/old2", 0, 2, 0, 2, 0x0, 0x0),
        MakeHistoryFileNode("del2/neq", 0, 3, 0, 3, 0x0, 0x0),
        MakeHistoryFileNode("del2/old1", 0, 4, 0, 4, 0x0, 0x0),
        MakeHistoryFileNode("del2/old2", 0, 5, 0, 5, 0x0, 0x0),
    };
    std::forward_list<FileNode> remoteNodes =
    {
        MakeFileNode("del1/neq", 0, 0, 0x0, 0x1),
        MakeFileNode("del1/meq", 0, 1, 0x0, 0x0),
        MakeFileNode("del1/mneq", 0, 2, 0x0, 0x1),
        // del2/neq
        // del2/old1
        // del2/old2
    };

    // OUT
    std::vector<FileNode> expectedNodes = 
    {
        MakeFileNode("del1/neq", 0, 0, 0x0, 0x1, FileNode::Dirty), // R
        MakeFileNode("del1/old1", 0, 1, 0x0, 0x0, FileNode::MovedClean), // R
        MakeFileNode("del1/old2", 0, 2, 0x0, 0x1, FileNode::MovedDirty), // R
        MakeFileNode("del2/neq", 0, 3, 0x0, 0x1, FileNode::Dirty), // L
        MakeFileNode("del2/old1", 0, 4, 0x0, 0x0, FileNode::MovedClean), // L
        MakeFileNode("del2/old2", 0, 5, 0x0, 0x1, FileNode::MovedDirty), // L
    };
    std::vector<HistoryFileNode> expectedHistory = 
    {
        MakeHistoryFileNode("del1/neq", 0, 0, 0, 0, 0x0, 0x0, FileNode::DeletedLocal),
        MakeHistoryFileNode("del1/old1", 0, 1, 0, 1, 0x0, 0x0, FileNode::DeletedLocal),
        MakeHistoryFileNode("del1/old2", 0, 2, 0, 2, 0x0, 0x0, FileNode::DeletedLocal),
        MakeHistoryFileNode("del2/neq", 0, 3, 0, 3, 0x0, 0x0, FileNode::DeletedRemote),
        MakeHistoryFileNode("del2/old1", 0, 4, 0, 4, 0x0, 0x0, FileNode::DeletedRemote),
        MakeHistoryFileNode("del2/old2", 0, 5, 0, 5, 0x0, 0x0, FileNode::DeletedRemote),
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("del2/neq", &expectedNodes[3], &expectedHistory[3], nullptr, PairedNode::Conflict),
        MakePairedNode("del2/meq", &expectedNodes[4], &expectedHistory[4], nullptr, PairedNode::Conflict),
        MakePairedNode("del2/mneq", &expectedNodes[5], &expectedHistory[5], nullptr, PairedNode::Conflict),
        MakePairedNode("del1/neq", nullptr, &expectedHistory[0], &expectedNodes[0], PairedNode::Conflict),
        MakePairedNode("del1/old1", nullptr, &expectedHistory[1], &expectedNodes[1], PairedNode::Conflict),
        MakePairedNode("del1/old2", nullptr, &expectedHistory[2], &expectedNodes[2], PairedNode::Conflict),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

/*
LOCAL               HISTORY             REMOTE
  mov1/               mov1/               mov1/
    meq (0x00)          old1(0x00)          old1(0x01)
    mne1(0x01)          old2(0x00)          old2(0x02)
    mne2(0x01)          old3(0x00)          new1(0x00)
    mne3(0x01)          old4(0x00)          old4(0x02)          
    mne4(0x01)          old5(0x00)          new2(0x00)
*/
TEST_F(PairingBasicTest, MovesLocal)
{
    // IN
    std::forward_list<FileNode> scanNodes =
    {
        MakeFileNode("mov1/meq", 0, 0, 0x0, 0x0),
        MakeFileNode("mov1/mne1", 0, 1, 0x0, 0x1),
        MakeFileNode("mov1/mne2", 0, 2, 0x0, 0x1),
        MakeFileNode("mov1/mne3", 0, 3, 0x0, 0x1),
        MakeFileNode("mov1/mne4", 0, 4, 0x0, 0x1),
    };
    std::forward_list<HistoryFileNode> historyNodes =
    {
        MakeHistoryFileNode("mov1/old1", 0, 0, 0, 0, 0x0, 0x0),
        MakeHistoryFileNode("mov1/old2", 0, 1, 0, 1, 0x0, 0x0),
        MakeHistoryFileNode("mov1/old3", 0, 2, 0, 2, 0x0, 0x0),
        MakeHistoryFileNode("mov1/old4", 0, 3, 0, 3, 0x0, 0x0),
        MakeHistoryFileNode("mov1/old5", 0, 4, 0, 4, 0x0, 0x0),
    };
    std::forward_list<FileNode> remoteNodes =
    {
        MakeFileNode("mov1/old1", 0, 0, 0x0, 0x1),
        MakeFileNode("mov1/old2", 0, 1, 0x0, 0x2),
        MakeFileNode("mov1/new1", 0, 2, 0x0, 0x0),
        MakeFileNode("mov1/old4", 0, 3, 0x0, 0x2),
        MakeFileNode("mov1/new2", 0, 4, 0x0, 0x0),
    };

    // OUT
    std::vector<FileNode> expectedNodes = 
    {
        MakeFileNode("mov1/meq", 0, 0, 0x0, 0x0, FileNode::MovedClean), // L
        MakeFileNode("mov1/old1", 0, 0, 0x0, 0x1, FileNode::Dirty), // R
        MakeFileNode("mov1/mne1", 0, 1, 0x0, 0x1, FileNode::MovedDirty), // L
        MakeFileNode("mov1/old2", 0, 1, 0x0, 0x2, FileNode::Dirty), // R
        MakeFileNode("mov1/mne2", 0, 2, 0x0, 0x1, FileNode::MovedDirty), // L
        MakeFileNode("mov1/new1", 0, 2, 0x0, 0x0, FileNode::MovedClean), // R
        MakeFileNode("mov1/mne3", 0, 3, 0x0, 0x1, FileNode::MovedDirty), // L
        MakeFileNode("mov1/old4", 0, 3, 0x0, 0x2, FileNode::Dirty), // R
        MakeFileNode("mov1/mne4", 0, 4, 0x0, 0x1, FileNode::MovedDirty), // L
        MakeFileNode("mov1/new2", 0, 4, 0x0, 0x0, FileNode::MovedClean), // R
    };
    std::vector<HistoryFileNode> expectedHistory = 
    {
        MakeHistoryFileNode("mov1/old1", 0, 0, 0, 0, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("mov1/old2", 0, 1, 0, 1, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("mov1/old3", 0, 2, 0, 2, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("mov1/old4", 0, 3, 0, 3, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("mov1/old5", 0, 4, 0, 4, 0x0, 0x0, FileNode::HistoryPresent),
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("mov1/meq", &expectedNodes[0], &expectedHistory[0], &expectedNodes[1], PairedNode::Conflict),
        MakePairedNode("mov1/mne1", &expectedNodes[2], &expectedHistory[1], &expectedNodes[3], PairedNode::Conflict),
        MakePairedNode("mov1/mne2", &expectedNodes[4], &expectedHistory[2], &expectedNodes[5], PairedNode::Conflict),
        MakePairedNode("mov1/mne3", &expectedNodes[6], &expectedHistory[3], &expectedNodes[7], PairedNode::Conflict),
        MakePairedNode("mov1/mne4", &expectedNodes[8], &expectedHistory[4], &expectedNodes[9], PairedNode::Conflict),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

/*
LOCAL               HISTORY             REMOTE
  mov2/               mov2/               mov2/
    old1(0x00)          old1(0x00)          meq (0x00)
    old2(0x02)          old2(0x00)          mne1(0x01)
    new1(0x00)          old3(0x00)          mne2(0x01)
    old4(0x02)          old4(0x00)          mne3(0x01)
    new2(0x00)          old5(0x00)          mne4(0x01)
*/
TEST_F(PairingBasicTest, MovesRemote)
{
    // IN
    std::forward_list<FileNode> scanNodes =
    {
        MakeFileNode("mov1/old1", 0, 0, 0x0, 0x1),
        MakeFileNode("mov1/old2", 0, 1, 0x0, 0x2),
        MakeFileNode("mov1/new1", 0, 2, 0x0, 0x0),
        MakeFileNode("mov1/old4", 0, 3, 0x0, 0x2),
        MakeFileNode("mov1/new2", 0, 4, 0x0, 0x0),
    };
    std::forward_list<HistoryFileNode> historyNodes =
    {
        MakeHistoryFileNode("mov1/old1", 0, 0, 0, 0, 0x0, 0x0),
        MakeHistoryFileNode("mov1/old2", 0, 1, 0, 1, 0x0, 0x0),
        MakeHistoryFileNode("mov1/old3", 0, 2, 0, 2, 0x0, 0x0),
        MakeHistoryFileNode("mov1/old4", 0, 3, 0, 3, 0x0, 0x0),
        MakeHistoryFileNode("mov1/old5", 0, 4, 0, 4, 0x0, 0x0),
    };
    std::forward_list<FileNode> remoteNodes =
    {
        MakeFileNode("mov1/meq", 0, 0, 0x0, 0x0),
        MakeFileNode("mov1/mne1", 0, 1, 0x0, 0x1),
        MakeFileNode("mov1/mne2", 0, 2, 0x0, 0x1),
        MakeFileNode("mov1/mne3", 0, 3, 0x0, 0x1),
        MakeFileNode("mov1/mne4", 0, 4, 0x0, 0x1),
    };

    // OUT
    std::vector<FileNode> expectedNodes = 
    {
        MakeFileNode("mov1/old1", 0, 0, 0x0, 0x1, FileNode::Dirty), // L
        MakeFileNode("mov1/meq", 0, 0, 0x0, 0x0, FileNode::MovedClean), // R
        MakeFileNode("mov1/old2", 0, 1, 0x0, 0x2, FileNode::Dirty), // L
        MakeFileNode("mov1/mne1", 0, 1, 0x0, 0x1, FileNode::MovedDirty), // R
        MakeFileNode("mov1/new1", 0, 2, 0x0, 0x0, FileNode::MovedClean), // L
        MakeFileNode("mov1/mne2", 0, 2, 0x0, 0x1, FileNode::MovedDirty), // R
        MakeFileNode("mov1/old4", 0, 3, 0x0, 0x2, FileNode::Dirty), // L
        MakeFileNode("mov1/mne3", 0, 3, 0x0, 0x1, FileNode::MovedDirty), // R
        MakeFileNode("mov1/new2", 0, 4, 0x0, 0x0, FileNode::MovedClean), // L
        MakeFileNode("mov1/mne4", 0, 4, 0x0, 0x1, FileNode::MovedDirty), // R
        
    };
    std::vector<HistoryFileNode> expectedHistory = 
    {
        MakeHistoryFileNode("mov1/old1", 0, 0, 0, 0, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("mov1/old2", 0, 1, 0, 1, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("mov1/old3", 0, 2, 0, 2, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("mov1/old4", 0, 3, 0, 3, 0x0, 0x0, FileNode::HistoryPresent),
        MakeHistoryFileNode("mov1/old5", 0, 4, 0, 4, 0x0, 0x0, FileNode::HistoryPresent),
    };
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("mov1/old1", &expectedNodes[0], &expectedHistory[0], &expectedNodes[1], PairedNode::Conflict),
        MakePairedNode("mov1/old2", &expectedNodes[2], &expectedHistory[1], &expectedNodes[3], PairedNode::Conflict),
        MakePairedNode("mov1/new1", &expectedNodes[4], &expectedHistory[2], &expectedNodes[5], PairedNode::Conflict),
        MakePairedNode("mov1/old4", &expectedNodes[6], &expectedHistory[3], &expectedNodes[7], PairedNode::Conflict),
        MakePairedNode("mov1/new2", &expectedNodes[8], &expectedHistory[4], &expectedNodes[9], PairedNode::Conflict),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

#undef EXPECT_ALL_NODES
