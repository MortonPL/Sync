#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include "Lib/General.h"
#include "Lib/Creeper.h"
#include "Lib/Blocker.h"
#include "Lib/Compressor.h"
#include "Lib/SSHConnector.h"
#include "Lib/Announcer.h"
#include "CLI/Global.h"
#include "CLI/CLIAnnouncer.h"
#include "Utils.h"

INITIALIZE_EASYLOGGINGPP

int readall(int fd, char* buffer, unsigned buffer_size)
{
    int bytes_left = buffer_size;
    int bytes_read = 0;
    while (bytes_left > 0)
    {
        bytes_read = read(fd, buffer + buffer_size - bytes_left, bytes_left);
        if (bytes_read == 0)
            break;
        if (bytes_read == -1)
            return -1;
        bytes_left -= bytes_read;
    }
    return buffer_size - bytes_left;
}

bool writeall(int fd, const char *buffer, unsigned buffer_size)
{
    int bytes_left = buffer_size;
    int bytes_written = 0;
    while (bytes_left > 0)
    {
        bytes_written = write(fd, buffer + buffer_size - bytes_left, bytes_left);
        bytes_left -= bytes_written;
        if (bytes_written <= 0)
            return false;
    }
    return true;
}

void ParseArgs(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] != '-')
            continue;

        switch (argv[i][1])
        {
        case 'c':   // _c_reep
            if (i + 1 >= argc)
                break;
            GlobalCLI::mode |= GlobalCLI::CLIMode::Creep;
            GlobalCLI::pathCreep = Utils::CorrectDirPath(argv[i+1]);
            break;
        case 'd':   // _d_aemon
            GlobalCLI::mode |= GlobalCLI::CLIMode::DaemonServant;
            break;
        case 'h':   // _h_ome & block
            GlobalCLI::mode |= GlobalCLI::CLIMode::HomePath;
            if (i + 1 < argc)
                GlobalCLI::pathBlock = Utils::CorrectDirPath(argv[i+1]);
            break;
        case 's':   // _s_tat
            if (i + 1 >= argc)
                break;
            GlobalCLI::mode |= GlobalCLI::CLIMode::Stat;
            GlobalCLI::pathStat = argv[i+1];
            break;
        case 'u':   // _u__nblock
            if (i + 1 >= argc)
                break;
            GlobalCLI::mode |= GlobalCLI::CLIMode::Unblock;
            GlobalCLI::pathUnblock = Utils::CorrectDirPath(argv[i+1]);
            break;
        case 'z':   // compress _z_std
            if (i + 2 >= argc)
                break;
            GlobalCLI::mode |= GlobalCLI::CLIMode::Compress;
            GlobalCLI::pathCompressIn = argv[i+1];
            GlobalCLI::pathCompressOut = argv[i+2];
            break;
        case 'Z':   // decompress _z_std
            if (i + 2 >= argc)
                break;
            GlobalCLI::mode |= GlobalCLI::CLIMode::Decompress;
            GlobalCLI::pathDecompressIn = argv[i+1];
            GlobalCLI::pathDecompressOut = argv[i+2];
            break;
        default:
            break;
        }
    }
}

void StatPath(std::string path)
{
    struct stat buf;
    if (stat(path.c_str(), &buf) != 0)
    {
        std::cout << (char)Messages::Error;
        std::cout.flush();
        return;
    }
    std::cout << (char)Messages::Stat;
    std::cout.flush();
    FileNode::MarshallingContainer buf2(FileNode::MiniStatBinarySize, FileNode::MarshallingUnit(0));
    FileNode::SerializeStat(&buf, buf2);
    writeall(1, (char*)buf2.data(), FileNode::MiniStatBinarySize);
}

void CreepDir(std::string path)
{
    std::cout << (char)Messages::Ok;
    std::cout.flush();

    std::forward_list<FileNode> nodes;
    auto creeper = Creeper();
    auto result = creeper.CreepPath(path, nodes);
    if (!Announcer::CreeperResult(result, CLIAnnouncer::Log))
    {
        std::cout << (char)Messages::Error;
        std::cout.flush();
        return;
    }
    std::cout << (char)Messages::Ok;
    std::cout.flush();

    const int timeout = 100 * 60; // TODO reduce back to 10 * 60
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    {
        tv = {timeout, 0};
        int retval = select(1, &rfds, nullptr, nullptr, &tv);
        if (retval == -1 || !retval)
        {
            LOG(ERROR) << "Client didn't respond in time.";
            return;
        }

        char ack;
        std::cin >> ack;
        if (ack != (char)Messages::Ok)
            return;
    }

    //start sending node data
    //prealocate decent size to avoid resizing later
    FileNode::MarshallingContainer buf;
    buf.reserve(PATH_MAX);
    std::size_t nnodes = creeper.GetResultsCount();
    LOG(INFO) << "Writing " << nnodes << " nodes.";
    writeall(1, (char*)&nnodes, sizeof(nnodes));
    std::size_t i = 0;
    for (auto& node: nodes)
    {
        if (i < 15000)
        {
            std::size_t size = node.Serialize(buf);
            writeall(1, (char*)buf.data(), size);
            i++;
        }
        else
        {
            int retval = select(1, &rfds, nullptr, nullptr, &tv);
            if (retval == -1 || !retval)
                return;

            char ack;
            std::cin >> ack;
            if (ack != (char)Messages::Ok)
                return;
            i = 0;
        }
    }
    LOG(INFO) << "Done.";
}

void GetHomePath()
{
    auto home = Utils::GetHomePath();
    unsigned short len = home.length();

    writeall(1, (char*)&len, sizeof(len));

    std::cout << home;
    std::cout.flush();
}

void BlockDir(std::string path)
{
    try
    {
        LOG(INFO) << "Blocking directory " << path;
        Blocker::Block(path);
        bool blocked = true;
        writeall(1, (char*)&blocked, sizeof(blocked));
    }
    catch (const Blocker::BlockerException&)
    {
        bool blocked = false;
        writeall(1, (char*)&blocked, sizeof(blocked));
    }
}

void UnblockDir(std::string path)
{
    try
    {
        LOG(INFO) << "Unblocking directory " << path;
        Blocker::Unblock(path);
        bool unblocked = true;
        writeall(1, (char*)&unblocked, sizeof(unblocked));
    }
    catch (const Blocker::BlockerException&)
    {
        bool unblocked = false;
        writeall(1, (char*)&unblocked, sizeof(unblocked));
    }
}

void Compress()
{
    off_t compressedSize = 0;
    std::cout << (char)Messages::Ok;
    std::cout.flush();
    if (Compressor::Compress(GlobalCLI::pathCompressIn, GlobalCLI::pathCompressOut, compressedSize))
    {
        writeall(1, (char*)&compressedSize, sizeof(compressedSize));
    }
    else
    {
        compressedSize = 0;
        writeall(1, (char*)&compressedSize, sizeof(compressedSize));
    }
}

void Decompress()
{
    std::cout << (char)Messages::Ok;
    std::cout << (Compressor::Decompress(GlobalCLI::pathDecompressIn, GlobalCLI::pathDecompressOut)? (char)Messages::Ok: (char)Messages::Error);
    std::cout.flush();
}

// NOTE: Unused
int ServeOnce(char)
{
    /*
    unsigned short pathSize;

    switch (mode)
    {
    case 's':
    {
        char buf[PATH_MAX];
        if (read(0, &pathSize, sizeof(pathSize)) != sizeof(pathSize))
            return 1;
        int len = pathSize > PATH_MAX? PATH_MAX: pathSize;
        if (read(0, buf, len) != len)
            return 1;
        StatPath(std::string(buf));
        break;
    }
    default:
        break;
    }
    */
    return 0;
}

// NOTE: Unused
int Serve()
{
    /*
    bool isDone = false;
    char mode = 0;

    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    std::cout << 'd';
    std::cout.flush();

    while (!isDone)
    {
        tv = {60, 0};
        int retval = select(1, &rfds, nullptr, nullptr, &tv);

        if (retval == -1)
        {
            break;
        }
        else if (retval)
        {
            if (read(0, &mode, sizeof(mode)) != sizeof(mode))
                break;
            if (mode != 0)
                if (ServeOnce(mode) != 0)
                    break;
        }
        else
        {
            mode = 0;
        }

        if (mode == 0)
            isDone = true;
    }

    std::cout << 0;
    std::cout.flush();
    */
    return 0;
}

int main(int argc, char* argv[])
{
    ParseArgs(argc, argv);

    if (GlobalCLI::mode & GlobalCLI::CLIMode::Stat)
    {
        StatPath(GlobalCLI::pathStat);
        return 0;
    }

    if (!General::InitEverything("synccli.log"))
        return -1;

    if (GlobalCLI::mode & GlobalCLI::CLIMode::Compress)
    {
        Compress();
        return 0;
    }

    if (GlobalCLI::mode & GlobalCLI::CLIMode::Decompress)
    {
        Decompress();
        return 0;
    }

    if (GlobalCLI::mode & GlobalCLI::CLIMode::HomePath)
    {
        GetHomePath();
        if (GlobalCLI::pathBlock != "")
            BlockDir(GlobalCLI::pathBlock);
    }

    if (GlobalCLI::mode & GlobalCLI::CLIMode::Unblock)
        UnblockDir(GlobalCLI::pathUnblock);

    if (GlobalCLI::mode & GlobalCLI::CLIMode::Creep)
        CreepDir(GlobalCLI::pathCreep);

    if (GlobalCLI::mode & GlobalCLI::CLIMode::DaemonServant)
    {
        Serve();
        return 0;
    }

    return 0;
}
