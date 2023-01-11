#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>
#include "Lib/Blocker.h"

class BlockingTest: public ::testing::Test
{
protected:
    BlockingTest()
    {
    };

    ~BlockingTest()
    {
    };
};

TEST_F(BlockingTest, BlockEmptyFile)
{
    const std::filesystem::path pathToBlock{"dir"};
    const std::filesystem::path blockFile{"block"};
    std::ofstream{pathToBlock};
    std::ofstream{blockFile};

    bool success = Blocker::Block(pathToBlock, blockFile);

    EXPECT_EQ(success, true);
    std::filesystem::remove_all(pathToBlock);
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, BlockNoFile)
{
    const std::filesystem::path pathToBlock{"dir"};
    const std::filesystem::path blockFile{"block"};
    std::ofstream{pathToBlock};

    bool success = Blocker::Block(pathToBlock, blockFile);

    EXPECT_EQ(success, true);
    std::filesystem::remove_all(pathToBlock);
}

TEST_F(BlockingTest, BlockNotExistingPath)
{
    const std::filesystem::path pathToBlock{"dir"};
    const std::filesystem::path blockFile{"block"};
    std::ofstream{blockFile};

    bool success = Blocker::Block(pathToBlock, blockFile);

    EXPECT_EQ(success, false);
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, BlockNoCollision)
{
    const std::filesystem::path pathToBlock{"dir"};
    const std::filesystem::path otherPath{"other"};
    const std::filesystem::path blockFile{"block"};
    std::filesystem::create_directories(pathToBlock);
    std::filesystem::create_directories(otherPath);
    std::ofstream{blockFile} << std::filesystem::canonical(otherPath).string() << std::endl;

    bool success = Blocker::Block(pathToBlock, blockFile);

    EXPECT_EQ(success, true);
    std::filesystem::remove_all(pathToBlock);
    std::filesystem::remove_all(otherPath);
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, BlockCollision)
{
    const std::filesystem::path pathToBlock{"dir"};
    const std::filesystem::path blockFile{"block"};
    std::filesystem::create_directories(pathToBlock);
    std::ofstream{blockFile} << std::filesystem::canonical(pathToBlock).string() << std::endl;

    bool success = Blocker::Block(pathToBlock, blockFile);

    EXPECT_EQ(success, false);
    std::filesystem::remove_all(pathToBlock);
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, BlockCollisionChild)
{
    const std::filesystem::path pathToBlock{"dir1"};
    const std::filesystem::path blockFile{"block"};
    std::filesystem::create_directories(pathToBlock/"dir2");
    std::ofstream{blockFile} << std::filesystem::canonical(pathToBlock/"dir2").string() << std::endl;

    bool success = Blocker::Block(pathToBlock, blockFile);

    EXPECT_EQ(success, false);
    std::filesystem::remove_all(pathToBlock);
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, BlockCollisionParent)
{
    const std::filesystem::path pathToBlock{"dir0/dir1"};
    const std::filesystem::path blockFile{"block"};
    std::filesystem::create_directories(pathToBlock);
    std::ofstream{blockFile} << std::filesystem::canonical("dir0").string() << std::endl;

    bool success = Blocker::Block(pathToBlock, blockFile);

    EXPECT_EQ(success, false);
    std::filesystem::remove_all("dir0");
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, UnblockEmptyFile)
{
    const std::filesystem::path pathToUnblock{"dir"};
    const std::filesystem::path blockFile{"block"};
    std::ofstream{pathToUnblock};
    std::ofstream{blockFile};

    bool success = Blocker::Unblock(pathToUnblock, blockFile);

    EXPECT_EQ(success, true);
    std::filesystem::remove_all(pathToUnblock);
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, UnblockNoFile)
{
    const std::filesystem::path pathToUnblock{"dir"};
    const std::filesystem::path blockFile{"block"};
    std::ofstream{pathToUnblock};

    bool success = Blocker::Unblock(pathToUnblock, blockFile);

    EXPECT_EQ(success, true);
    std::filesystem::remove_all(pathToUnblock);
}

TEST_F(BlockingTest, UnblockNotExistingPath)
{
    const std::filesystem::path pathToUnblock{"dir"};
    const std::filesystem::path blockFile{"block"};
    std::ofstream{blockFile};

    bool success = Blocker::Unblock(pathToUnblock, blockFile);

    EXPECT_EQ(success, false);
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, UnblockNoMatch)
{
    const std::filesystem::path pathToBlock{"dir"};
    const std::filesystem::path otherPath{"other"};
    const std::filesystem::path blockFile{"block"};
    std::filesystem::create_directories(pathToBlock);
    std::filesystem::create_directories(otherPath);
    std::ofstream{blockFile} << std::filesystem::canonical(otherPath).string() << std::endl;

    bool success = Blocker::Unblock(pathToBlock, blockFile);

    EXPECT_EQ(success, true);
    std::filesystem::remove_all(pathToBlock);
    std::filesystem::remove_all(otherPath);
    std::filesystem::remove(blockFile);
}

TEST_F(BlockingTest, UnblockMatch)
{
    const std::filesystem::path pathToBlock{"dir"};
    const std::filesystem::path blockFile{"block"};
    std::filesystem::create_directories(pathToBlock);
    std::ofstream{blockFile} << std::filesystem::canonical(pathToBlock).string() << std::endl;

    bool success = Blocker::Unblock(pathToBlock, blockFile);

    EXPECT_EQ(success, true);
    std::filesystem::remove_all(pathToBlock);
    std::filesystem::remove(blockFile);
}
