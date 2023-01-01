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
        std::filesystem::remove_all("sandbox");
    };

    ~CreepingTest()
    {
        std::filesystem::remove_all("sandbox");
    };

    std::forward_list<FileNode> scanNodes;
    Creeper creeper;
};

void ExpectFileNode(FileNode* node, FileNode* expected)
{
    EXPECT_NE(node, nullptr) << "FileNode does not exist!";
    EXPECT_NE(expected, nullptr) << "Too many FileNodes!";
    EXPECT_EQ(node->path, expected->path) << "FileNode " << expected->path << " has wrong path!";
}

#define EXPECT_ALL_NODES(scanNodes, expectedResult)\
{\
    auto sit = scanNodes.begin();\
    auto eit = expectedResult.begin();\
    for (int i = 0; i < expectedResult.size(); i++)\
    {\
        ExpectFileNode(sit != scanNodes.end()? &*sit: nullptr, eit != expectedResult.end()? &*eit: nullptr);\
        sit++;\
        eit++;\
    }\
}

TEST_F(CreepingTest, Empty)
{
    const std::filesystem::path sandbox{"sandbox"};
    std::filesystem::create_directory(sandbox);

    int rc = creeper.CreepPath(sandbox, scanNodes);

    EXPECT_EQ(rc, CREEP_OK);
    EXPECT_EQ(scanNodes.begin(), scanNodes.end());
    EXPECT_EQ(creeper.GetResultsCount(), 0);
}

TEST_F(CreepingTest, NoRoot)
{
    int rc = creeper.CreepPath("sandbox", scanNodes);

    EXPECT_EQ(rc, CREEP_EXIST);
    EXPECT_EQ(scanNodes.begin(), scanNodes.end());
    EXPECT_EQ(creeper.GetResultsCount(), 0);
}

TEST_F(CreepingTest, RootNotADirectory)
{
    const std::filesystem::path sandbox{"sandbox"};
    std::ofstream{sandbox};

    int rc = creeper.CreepPath("sandbox", scanNodes);

    EXPECT_EQ(rc, CREEP_NOTDIR);
    EXPECT_EQ(scanNodes.begin(), scanNodes.end());
    EXPECT_EQ(creeper.GetResultsCount(), 0);
}

TEST_F(CreepingTest, RootWrongPermissions)
{
    const std::filesystem::path sandbox{"sandboxProtected"};
    std::filesystem::create_directory(sandbox);
    std::ofstream{sandbox/"a"};
    std::filesystem::permissions(sandbox, std::filesystem::perms::owner_exec, std::filesystem::perm_options::remove);

    int rc = creeper.CreepPath(sandbox, scanNodes);

    EXPECT_EQ(rc, CREEP_PERM);
    EXPECT_EQ(scanNodes.begin(), scanNodes.end());
    EXPECT_EQ(creeper.GetResultsCount(), 0);
}

TEST_F(CreepingTest, FlatDirectory)
{
    const std::filesystem::path sandbox{"sandbox"};
    std::filesystem::create_directory(sandbox);
    std::ofstream{sandbox/"a"};
    std::ofstream{sandbox/"b"};
    std::ofstream{sandbox/"c"};

    std::list<FileNode> expectedResult =
    {
        FileNode("c"),
        FileNode("b"),
        FileNode("a"),
    };

    int rc = creeper.CreepPath(sandbox.string() + "/", scanNodes);

    EXPECT_EQ(rc, CREEP_OK);
    EXPECT_EQ(creeper.GetResultsCount(), 3);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

TEST_F(CreepingTest, DeepDirectory)
{
    const std::filesystem::path sandbox{"sandbox"};
    std::filesystem::create_directory(sandbox);
    std::filesystem::create_directories(sandbox/"dir1");
    std::filesystem::create_directories(sandbox/"dir2"/"dir3");
    std::filesystem::create_directories(sandbox/"emptydir");
    std::ofstream{sandbox/"a"};
    std::ofstream{sandbox/"dir1"/"b"};
    std::ofstream{sandbox/"dir2"/"dir3"/"c"};

    std::list<FileNode> expectedResult =
    {
        // no emptydir!
        FileNode("dir2/dir3/c"),
        FileNode("dir1/b"),
        FileNode("a"),
    };

    int rc = creeper.CreepPath(sandbox.string() + "/", scanNodes);

    EXPECT_EQ(rc, CREEP_OK);
    EXPECT_EQ(creeper.GetResultsCount(), 3);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

TEST_F(CreepingTest, IgnoreSymLinks)
{
    const std::filesystem::path sandbox{"sandbox"};
    std::filesystem::create_directory(sandbox);
    std::ofstream{sandbox/"a"};
    std::filesystem::create_symlink(sandbox/"a", sandbox/"link");

    std::list<FileNode> expectedResult =
    {
        // no link!
        FileNode("a"),
    };

    int rc = creeper.CreepPath(sandbox.string() + "/", scanNodes);

    EXPECT_EQ(rc, CREEP_OK);
    EXPECT_EQ(creeper.GetResultsCount(), 1);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

// note - hard links can cause funny effects, and only the first of them (last alphabetically) will be mapped
TEST_F(CreepingTest, HardLinks)
{
    const std::filesystem::path sandbox{"sandbox"};
    std::filesystem::create_directory(sandbox);
    std::filesystem::create_directories(sandbox/"dir1");
    std::ofstream{sandbox/"a"};
    std::filesystem::create_hard_link(sandbox/"a", sandbox/"link");
    std::filesystem::create_hard_link(sandbox/"a", sandbox/"link2");
    std::filesystem::create_hard_link(sandbox/"a", sandbox/"dir1"/"link3");

    std::list<FileNode> expectedResult =
    {
        FileNode("link2"),
        FileNode("link"),
        FileNode("dir1/link3"),
        FileNode("a"),
    };

    int rc = creeper.CreepPath(sandbox.string() + "/", scanNodes);

    EXPECT_EQ(rc, CREEP_OK);
    EXPECT_EQ(creeper.GetResultsCount(), 4);
    EXPECT_ALL_NODES(scanNodes, expectedResult);
}

#undef EXPECT_ALL_NODES
