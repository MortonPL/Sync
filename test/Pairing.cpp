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
    HistoryFileNode h(path, dev, inode, rdev, rinode, 0, 0, 0, hashHigh, hashLow);
    h.status = status;
    return h;
}

PairedNode MakePairedNode(std::string path, FileNode::Status local, FileNode::Status history, FileNode::Status remote, PairedNode::Action action=PairedNode::Action::None)
{
    auto l = FileNode();
    l.status = local;
    auto h = HistoryFileNode();
    h.status = history;
    auto r = FileNode();
    r.status = remote;
    PairedNode p(path, l, h, r);
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

#define EXPECT_PAIRED_NODE(pair, expected)\
{\
    PairedNode* pPair = pair;\
    PairedNode* pExpected = expected;\
    EXPECT_NE(pPair, nullptr) << "PairedNode does not exist!";\
    EXPECT_NE(pExpected, nullptr) << "Too many PairedNodes!";\
    EXPECT_EQ(pPair->path, pExpected->path) << "PairedNode " << pExpected->path << " has wrong path!";\
    EXPECT_EQ(pPair->action, pExpected->action) << "PairedNode " << pExpected->path << " has wrong action!";\
    EXPECT_EQ(pPair->defaultAction, pExpected->defaultAction) << "PairedNode " << pExpected->path << " has wrong default action!";\
    if (!pExpected->localNode.IsEmpty())\
        EXPECT_EQ(pPair->localNode.status, pExpected->localNode.status) << "PairedNode " << pExpected->path << " has local status mismatch!";\
    else\
        EXPECT_EQ(pPair->localNode.IsEmpty(), true);\
    if (!pExpected->historyNode.IsEmpty())\
        EXPECT_EQ(pPair->historyNode.status, pExpected->historyNode.status) << "PairedNode " << pExpected->path << " has history status mismatch!";\
    else\
        EXPECT_EQ(pPair->historyNode.IsEmpty(), true);\
    if (!pExpected->remoteNode.IsEmpty())\
        EXPECT_EQ(pPair->remoteNode.status, pExpected->remoteNode.status) << "PairedNode " << pExpected->path << " has remote status mismatch!";\
    else\
        EXPECT_EQ(pPair->remoteNode.IsEmpty(), true);\
}

//COMMONLY USED NAMES IN TESTS:
//    eq - equal (clean)
//    neq - not equal (dirty)
//    meq - moved equal (moved clean)
//    mneq - moved not equal (moved dirty)
//    old - saved in history
//VALUES IN ( ) ARE HASH VALUES.
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
        EXPECT_PAIRED_NODE(pit != pairedNodes.end()? &*pit: nullptr, eit != expectedResult.end()? &*eit: nullptr);\
        pit++;\
        eit++;\
    }\
}

//LOCAL               HISTORY             REMOTE
//  noh/                noh/                noh/
//    eq (0x00)           -                   eq (0x00)
//    neq(0x00)           -                   neq(0x01)
//    loc(0x00)           -                   -
//    -                   -                   rem(0x00)
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
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("noh/eq", FileNode::New, FileNode::Absent, FileNode::New, PairedNode::FastForward),
        MakePairedNode("noh/neq", FileNode::New, FileNode::Absent, FileNode::New, PairedNode::Conflict),
        MakePairedNode("noh/loc", FileNode::New, FileNode::Absent, FileNode::Absent, PairedNode::LocalToRemote),
        MakePairedNode("noh/rem", FileNode::Absent, FileNode::Absent, FileNode::New, PairedNode::RemoteToLocal),
    };

    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}



//LOCAL               HISTORY             REMOTE
//  ltor/               ltor/               ltor/
//    neq (0x01)          neq (0x00)          neq (0x00)
//    meq (0x00)          old1(0x00)          old1(0x00)
//    mneq(0x01)          old2(0x00)          old2(0x00)
//    -                   old3(0x00)          old3(0x00)
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
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("ltor/neq", FileNode::Dirty, FileNode::HistoryPresent, FileNode::Clean, PairedNode::LocalToRemote),
        MakePairedNode("ltor/meq", FileNode::New, FileNode::Absent, FileNode::Absent, PairedNode::LocalToRemote),
        MakePairedNode("ltor/mneq", FileNode::New, FileNode::Absent, FileNode::Absent, PairedNode::LocalToRemote),
        MakePairedNode("ltor/old1", FileNode::Absent, FileNode::HistoryPresent, FileNode::Clean, PairedNode::LocalToRemote),
        MakePairedNode("ltor/old2", FileNode::Absent, FileNode::HistoryPresent, FileNode::Clean, PairedNode::LocalToRemote),
        MakePairedNode("ltor/old3", FileNode::Absent, FileNode::HistoryPresent, FileNode::Clean, PairedNode::LocalToRemote),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

//LOCAL               HISTORY             REMOTE
//  rtol/               rtol/               rtol/
//    neq (0x00)          neq (0x00)          neq (0x01)
//    old1(0x00)          old1(0x00)          meq (0x00)
//    old2(0x00)          old2(0x00)          mneq(0x01)
//    old3(0x00)          old3(0x00)          -
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
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("rtol/neq", FileNode::Clean, FileNode::HistoryPresent, FileNode::Dirty, PairedNode::RemoteToLocal),
        MakePairedNode("rtol/old1", FileNode::Clean, FileNode::HistoryPresent, FileNode::Absent, PairedNode::RemoteToLocal),
        MakePairedNode("rtol/old2", FileNode::Clean, FileNode::HistoryPresent, FileNode::Absent, PairedNode::RemoteToLocal),
        MakePairedNode("rtol/old3", FileNode::Clean, FileNode::HistoryPresent, FileNode::Absent, PairedNode::RemoteToLocal),
        MakePairedNode("rtol/meq", FileNode::Absent, FileNode::Absent, FileNode::New, PairedNode::RemoteToLocal),
        MakePairedNode("rtol/mneq", FileNode::Absent, FileNode::Absent, FileNode::New, PairedNode::RemoteToLocal),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

//LOCAL               HISTORY             REMOTE
//  ffwd/               ffwd/               ffwd/
//    eq  (0x00)          eq  (0x00)          eq  (0x00)
//    neq (0x01)          neq (0x00)          neq (0x01)
//    meq (0x00)          old1(0x00)          meq (0x00)
//    mneq(0x01)          old2(0x00)          mneq(0x01)
//    -                   old3(0x00)          -
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
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("ffwd/eq", FileNode::Clean, FileNode::HistoryPresent, FileNode::Clean, PairedNode::DoNothing),
        MakePairedNode("ffwd/neq", FileNode::Dirty, FileNode::HistoryPresent, FileNode::Dirty, PairedNode::FastForward),
        MakePairedNode("ffwd/meq", FileNode::New, FileNode::Absent, FileNode::New, PairedNode::FastForward),
        MakePairedNode("ffwd/mneq", FileNode::New, FileNode::Absent, FileNode::New, PairedNode::FastForward),
        MakePairedNode("ffwd/old1", FileNode::Absent, FileNode::HistoryPresent, FileNode::Absent, PairedNode::FastForward),
        MakePairedNode("ffwd/old2", FileNode::Absent, FileNode::HistoryPresent, FileNode::Absent, PairedNode::FastForward),
        MakePairedNode("ffwd/old3", FileNode::Absent, FileNode::HistoryPresent, FileNode::Absent, PairedNode::FastForward),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

//LOCAL               HISTORY             REMOTE
//  del1/               del1/               del1/
//    -                   neq (0x00)          neq (0x01)
//    -                   old1(0x00)          meq (0x00)
//    -                   old2(0x00)          mneq(0x01)
//  del2/               del2/               del2/
//    neq (0x01)          neq (0x00)          -
//    meq (0x00)          old1(0x00)          -
//    mneq(0x01)          old2(0x00)          -
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
    std::list<PairedNode> expectedResult = 
    {
        MakePairedNode("del2/neq", FileNode::Dirty, FileNode::HistoryPresent, FileNode::Absent, PairedNode::Conflict),
        MakePairedNode("del2/meq", FileNode::New, FileNode::Absent, FileNode::Absent, PairedNode::LocalToRemote),
        MakePairedNode("del2/mneq", FileNode::New, FileNode::Absent, FileNode::Absent, PairedNode::LocalToRemote),
        MakePairedNode("del1/neq", FileNode::Absent, FileNode::HistoryPresent, FileNode::Dirty, PairedNode::Conflict),
        MakePairedNode("del1/old1", FileNode::Absent, FileNode::HistoryPresent, FileNode::Absent, PairedNode::FastForward),
        MakePairedNode("del1/old2", FileNode::Absent, FileNode::HistoryPresent, FileNode::Absent, PairedNode::FastForward),
        MakePairedNode("del2/old1", FileNode::Absent, FileNode::HistoryPresent, FileNode::Absent, PairedNode::FastForward),
        MakePairedNode("del2/old2", FileNode::Absent, FileNode::HistoryPresent, FileNode::Absent, PairedNode::FastForward),
        MakePairedNode("del1/meq", FileNode::Absent, FileNode::Absent, FileNode::New, PairedNode::RemoteToLocal),
        MakePairedNode("del1/mneq", FileNode::Absent, FileNode::Absent, FileNode::New, PairedNode::RemoteToLocal),
    };
    
    // run
    PairingManager::PairAll(scanNodes, historyNodes, remoteNodes, pairedNodes, emptyCreeper, mapper);
    // asserts
    EXPECT_EQ(pairedNodes.size(), expectedResult.size());
    EXPECT_ALL_NODES(pairedNodes, expectedResult);
}

#undef EXPECT_ALL_NODES
