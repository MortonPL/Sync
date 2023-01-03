#include <gtest/gtest.h>

#include <vector>
#include "Lib/Mapper.h"

class MappingTest: public ::testing::Test
{
protected:
    MappingTest()
    {
    };

    ~MappingTest()
    {
    };

    Mapper mapper;
};

FileNode MakeFileNode(std::string path, dev_t dev, ino_t inode)
{
    return FileNode(path, dev, inode, 0, 0, 0, 0);
}

void EmplacePaths(Mapper& mapper, std::vector<PairedNode>& nodes)
{
    for (auto& node: nodes)
    {
        mapper.EmplaceMapPath(node.path, node);
    }
}

void EmplaceLocalInodes(Mapper& mapper, std::vector<PairedNode>& nodes)
{
    for (auto& node: nodes)
    {
        mapper.EmplaceMapLocalInode(node.localNode.GetDevInode(), node);
    }
}

void EmplaceRemoteInodes(Mapper& mapper, std::vector<PairedNode>& nodes)
{
    for (auto& node: nodes)
    {
        mapper.EmplaceMapRemoteInode(node.remoteNode.GetDevInode(), node);
    }
}

#define EXPECT_ALL_PATHS(mapper, expectedResult)\
{\
    for (auto& pair: expectedResult)\
    {\
        EXPECT_EQ(mapper.FindMapPath(pair.first), pair.second);\
    }\
}

#define EXPECT_ALL_LOCAL_INODES(mapper, expectedResult)\
{\
    for (auto& pair: expectedResult)\
    {\
        EXPECT_EQ(mapper.FindMapLocalInode(pair.first), pair.second);\
    }\
}

#define EXPECT_ALL_REMOTE_INODES(mapper, expectedResult)\
{\
    for (auto& pair: expectedResult)\
    {\
        EXPECT_EQ(mapper.FindMapRemoteInode(pair.first), pair.second);\
    }\
}

TEST_F(MappingTest, SimplePath)
{
    std::vector<PairedNode> nodes =
    {
        PairedNode("a"),
        PairedNode("b"),
    };

    std::vector<std::pair<std::string, PairedNode*>> expectedResult =
    {
        {"a", &nodes[0]},
        {"b", &nodes[1]},
    };

    EmplacePaths(mapper, nodes);

    EXPECT_ALL_PATHS(mapper, expectedResult);
}

TEST_F(MappingTest, SimpleLocalInode)
{
    std::vector<FileNode> nodes =
    {
        MakeFileNode("", 0, 0),
        MakeFileNode("", 0, 1),
    };

    std::vector<PairedNode> pairedNodes =
    {
        PairedNode("", nodes[0]),
        PairedNode("", nodes[1]),
    };

    std::vector<std::pair<FileNode::devinode, PairedNode*>> expectedResult =
    {
        {{0, 0}, &pairedNodes[0]},
        {{0, 1}, &pairedNodes[1]},
    };

    EmplaceLocalInodes(mapper, pairedNodes);

    EXPECT_ALL_LOCAL_INODES(mapper, expectedResult);
}

TEST_F(MappingTest, SimpleRemoteInode)
{
    std::vector<FileNode> nodes =
    {
        MakeFileNode("", 0, 0),
        MakeFileNode("", 0, 1),
    };

    std::vector<PairedNode> pairedNodes =
    {
        PairedNode("", FileNode(), HistoryFileNode(), nodes[0]),
        PairedNode("", FileNode(), HistoryFileNode(), nodes[1]),
    };

    std::vector<std::pair<FileNode::devinode, PairedNode*>> expectedResult =
    {
        {{0, 0}, &pairedNodes[0]},
        {{0, 1}, &pairedNodes[1]},
    };

    EmplaceRemoteInodes(mapper, pairedNodes);

    EXPECT_ALL_REMOTE_INODES(mapper, expectedResult);
}

TEST_F(MappingTest, InodeCollision)
{
    std::vector<FileNode> nodes =
    {
        MakeFileNode("", 0, 0),
        MakeFileNode("a", 0, 1),
        MakeFileNode("b", 0, 1),
    };

    std::vector<PairedNode> pairedNodes =
    {
        PairedNode("", nodes[0]),
        PairedNode("", nodes[1]),
        PairedNode("", nodes[2]),
    };

    std::vector<std::pair<FileNode::devinode, PairedNode*>> expectedResult =
    {
        {{0, 0}, &pairedNodes[0]},
        {{0, 1}, &pairedNodes[1]},
    };

    EmplaceLocalInodes(mapper, pairedNodes);

    EXPECT_ALL_LOCAL_INODES(mapper, expectedResult);
}
