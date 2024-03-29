#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <filesystem>
#include <list>
#include <forward_list>
#include "Lib/Creeper.h"


class CreepingTest: public ::testing::Test
{
protected:
    CreepingTest()
    {
        std::filesystem::remove_all(sandbox);
    };

    ~CreepingTest()
    {
        std::filesystem::remove_all(sandbox);
    };

    std::forward_list<FileNode> scanNodes;
    Creeper creeper;
    const std::filesystem::path sandbox{"sandbox"};
};

#define EXPECT_FILE_NODE(node, expected)\
{\
    FileNode* pNode = node;\
    FileNode* pExpected = expected;\
    EXPECT_NE(pNode, nullptr) << "FileNode does not exist!";\
    EXPECT_NE(pExpected, nullptr) << "Too many FileNodes!";\
    EXPECT_EQ(pNode->path, pExpected->path) << "FileNode " << pExpected->path << " has wrong path!";\
}

#define EXPECT_ALL_NODES(scanNodes, expectedResult)\
{\
    auto sit = scanNodes.begin();\
    auto eit = expectedResult.begin();\
    for (int i = 0; i < expectedResult.size(); i++)\
    {\
        EXPECT_FILE_NODE(sit != scanNodes.end()? &*sit: nullptr, eit != expectedResult.end()? &*eit: nullptr);\
        sit++;\
        eit++;\
    }\
}

TEST_F(CreepingTest, Empty)
{
    std::filesystem::create_directory(sandbox);

    auto result = creeper.CreepPath(sandbox, scanNodes);

    EXPECT_EQ(result, Creeper::Result::Ok);
    EXPECT_EQ(scanNodes.begin(), scanNodes.end());
    EXPECT_EQ(creeper.GetResultsCount(), 0);
}

TEST_F(CreepingTest, NoRoot)
{
    auto result = creeper.CreepPath(sandbox, scanNodes);

    EXPECT_EQ(result, Creeper::Result::NotExists);
    EXPECT_EQ(scanNodes.begin(), scanNodes.end());
    EXPECT_EQ(creeper.GetResultsCount(), 0);
}

TEST_F(CreepingTest, RootNotADirectory)
{
    std::ofstream{sandbox};

    auto result = creeper.CreepPath(sandbox, scanNodes);

    EXPECT_EQ(result, Creeper::Result::NotADir);
    EXPECT_EQ(scanNodes.begin(), scanNodes.end());
    EXPECT_EQ(creeper.GetResultsCount(), 0);
}

TEST_F(CreepingTest, RootWrongPermissions)
{
    std::filesystem::create_directory(sandbox);
    std::filesystem::permissions(sandbox, std::filesystem::perms::owner_all, std::filesystem::perm_options::remove);

    auto result = creeper.CreepPath(sandbox, scanNodes);

    std::filesystem::permissions(sandbox, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);

    EXPECT_EQ(result, Creeper::Result::Permissions);
    EXPECT_EQ(scanNodes.begin(), scanNodes.end());
    EXPECT_EQ(creeper.GetResultsCount(), 0);
}

TEST_F(CreepingTest, FlatDirectory)
{
    std::filesystem::create_directory(sandbox);
    std::ofstream{sandbox/"a"};
    std::ofstream{sandbox/"b"};
    std::ofstream{sandbox/"c"};

    std::list<FileNode> expectedResult =
    {
        FileNode("a"),
        FileNode("b"),
        FileNode("c"),
    };

    auto result = creeper.CreepPath(sandbox.string() + "/", scanNodes);
    scanNodes.sort();

    EXPECT_EQ(result, Creeper::Result::Ok);
    EXPECT_EQ(creeper.GetResultsCount(), 3);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

TEST_F(CreepingTest, DeepDirectory)
{
    std::filesystem::create_directory(sandbox);
    std::filesystem::create_directories(sandbox/"dir1");
    std::filesystem::create_directories(sandbox/"dir2"/"dir3");
    std::filesystem::create_directories(sandbox/"emptydir");
    std::ofstream{sandbox/"a"};
    std::ofstream{sandbox/"dir1"/"b"};
    std::ofstream{sandbox/"dir2"/"dir3"/"c"};

    std::list<FileNode> expectedResult =
    {
        FileNode("a"),
        FileNode("dir1/b"),
        FileNode("dir2/dir3/c"),
        // no emptydir!
    };

    auto result = creeper.CreepPath(sandbox.string() + "/", scanNodes);
    scanNodes.sort();

    EXPECT_EQ(result, Creeper::Result::Ok);
    EXPECT_EQ(creeper.GetResultsCount(), 3);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

TEST_F(CreepingTest, IgnoreSymLinks)
{
    std::filesystem::create_directory(sandbox);
    std::ofstream{sandbox/"a"};
    std::filesystem::create_symlink(sandbox/"a", sandbox/"link");

    std::list<FileNode> expectedResult =
    {
        FileNode("a"),
        // no link!
    };

    auto result = creeper.CreepPath(sandbox.string() + "/", scanNodes);
    scanNodes.sort();

    EXPECT_EQ(result, Creeper::Result::Ok);
    EXPECT_EQ(creeper.GetResultsCount(), 1);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

// note - hard links can cause funny effects, and only the first of them (last alphabetically) will be mapped
TEST_F(CreepingTest, HardLinks)
{
    std::filesystem::create_directory(sandbox);
    std::filesystem::create_directories(sandbox/"dir1");
    std::ofstream{sandbox/"a"};
    std::filesystem::create_hard_link(sandbox/"a", sandbox/"link");
    std::filesystem::create_hard_link(sandbox/"a", sandbox/"link2");
    std::filesystem::create_hard_link(sandbox/"a", sandbox/"dir1"/"link3");

    std::list<FileNode> expectedResult =
    {
        FileNode("a"),
        FileNode("dir1/link3"),
        FileNode("link"),
        FileNode("link2"),     
    };

    auto result = creeper.CreepPath(sandbox.string() + "/", scanNodes);
    scanNodes.sort();

    EXPECT_EQ(result, Creeper::Result::Ok);
    EXPECT_EQ(creeper.GetResultsCount(), 4);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

TEST_F(CreepingTest, SyncBlackListPresent)
{
    std::filesystem::create_directory(sandbox);
    std::filesystem::create_directories(sandbox/"dir1");
    std::filesystem::create_directories(sandbox/"dir2"/"dir3");
    std::ofstream{sandbox/Creeper::SyncBlackListFile} << "dir*";
    std::ofstream{sandbox/"a"};
    std::ofstream{sandbox/"dir1"/"b"};
    std::ofstream{sandbox/"dir2"/"dir3"/"c"};

    std::list<FileNode> expectedResult =
    {
        FileNode(Creeper::SyncBlackListFile),
        FileNode("a"),
        // no dir*!
    };

    auto result = creeper.CreepPath(sandbox.string() + "/", scanNodes);
    scanNodes.sort();

    EXPECT_EQ(result, Creeper::Result::Ok);
    EXPECT_EQ(creeper.GetResultsCount(), 2);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

TEST_F(CreepingTest, SyncWhiteListPresent)
{
    std::filesystem::create_directory(sandbox);
    std::filesystem::create_directories(sandbox/"dir1");
    std::filesystem::create_directories(sandbox/"dir2"/"dir3");
    std::ofstream{sandbox/Creeper::SyncBlackListFile} << "dir*";
    std::ofstream{sandbox/Creeper::SyncWhiteListFile} << "dir2/dir3/*";
    std::ofstream{sandbox/"a"};
    std::ofstream{sandbox/"dir1"/"b"};
    std::ofstream{sandbox/"dir2"/"dir3"/"c"};

    std::list<FileNode> expectedResult =
    {
        FileNode(Creeper::SyncBlackListFile),
        FileNode(Creeper::SyncWhiteListFile),
        FileNode("a"),
        // no dir*!
        FileNode("dir2/dir3/c"),
    };

    auto result = creeper.CreepPath(sandbox.string() + "/", scanNodes);
    scanNodes.sort();

    EXPECT_EQ(result, Creeper::Result::Ok);
    EXPECT_EQ(creeper.GetResultsCount(), 4);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

#undef EXPECT_ALL_NODES
