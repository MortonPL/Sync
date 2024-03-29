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
        std::cout << 1;
        std::cout.flush();
        return;
    }
    std::cout << 's';
    std::cout.flush();
    unsigned char* buf2 = new unsigned char[FileNode::MiniStatBinarySize];
    FileNode::SerializeStat(&buf, buf2);
    writeall(1, (char*)buf2, FileNode::MiniStatBinarySize);
    delete[] buf2;
}

void CreepDir(std::string path)
{
    std::cout << '0';
    std::cout.flush();

    std::forward_list<FileNode> nodes;
    auto creeper = Creeper();
    auto result = creeper.CreepPath(path, nodes);
    if (!Announcer::CreeperResult(result, CLIAnnouncer::Log))
    {
        std::cout << '1'; // alternative to writeall --- in text mode
        std::cout.flush();
        return;
    }
    std::cout << '0';
    std::cout.flush();

    const int timeout = 300;
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    {
        tv = {timeout, 0};
        int retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval == -1 || !retval)
            return;

        char ack;
        std::cin >> ack;
        if (ack != '0')
            return;
    }

    //start sending node data
    unsigned char* buf = new unsigned char[FileNode::MaxBinarySize];
    std::size_t nnodes = creeper.GetResultsCount();
    LOG(INFO) << "Writing " << nnodes << " nodes.";
    writeall(1, (char*)&nnodes, sizeof(nnodes));
    for (auto& node: nodes)
    {
        unsigned short size = node.Serialize(buf);
        writeall(1, (char*)buf, size);
    }
    LOG(INFO) << "Done.";
    delete[] buf;
}

int GetHomePath()
{
    auto home = Utils::GetHomePath();
    unsigned short len = home.length();

    writeall(1, (char*)&len, sizeof(len));

    std::cout << home;
    std::cout.flush();

    return 0;
}

int BlockDir(std::string path)
{
    LOG(INFO) << "Blocking directory " << path;
    bool blocked = Blocker::Block(path);
    writeall(1, (char*)&blocked, sizeof(blocked));
    return 0;
}

int UnblockDir(std::string path)
{
    LOG(INFO) << "Unblocking directory " << path;
    bool unblocked = Blocker::Unblock(path);
    writeall(1, (char*)&unblocked, sizeof(unblocked));
    return 0;
}

int ServeOnce(char mode)
{
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

    return 0;
}

int Serve()
{
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
        int retval = select(1, &rfds, NULL, NULL, &tv);

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
        off_t compressedSize = 0;
        std::cout << 0;
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
        return 0;
    }

    if (GlobalCLI::mode & GlobalCLI::CLIMode::Decompress)
    {
        std::cout << 0;
        std::cout << (Compressor::Decompress(GlobalCLI::pathDecompressIn, GlobalCLI::pathDecompressOut)? 0: 1);
        std::cout.flush();
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
