#include <sys/stat.h>
#include <unistd.h>

#include "Lib/General.h"
#include "Lib/Creeper.h"
#include "Lib/Blocker.h"
#include "Lib/SSHConnector.h"
#include "Lib/SocketListener.h"
#include "Lib/Announcer.h"
#include "CLI/Global.h"
#include "CLI/CLIAnnouncer.h"
#include "Utils.h"

INITIALIZE_EASYLOGGINGPP

void ParseArgs(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] != '-')
            continue;

        switch (argv[i][1])
        {
        case 'c':
            if (i + 1 >= argc)
                break;
            GlobalCLI::dirToCreep = Utils::CorrectDirPath(argv[i+1]);
            break;
        case 'd':
            GlobalCLI::mode |= GlobalCLI::CLIMode::DaemonServant;
            break;
        case 'h':
            GlobalCLI::mode |= GlobalCLI::CLIMode::HomePath;
            if (i + 1 < argc)
                GlobalCLI::dirToBlock = Utils::CorrectDirPath(argv[i+1]);
            break;
        case 's':
            if (i + 1 >= argc)
                break;
            GlobalCLI::pathToStat = argv[i+1];
            break;
        case 'u':
            if (i + 1 >= argc)
                break;
            GlobalCLI::dirToUnblock = Utils::CorrectDirPath(argv[i+1]);
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
    std::cout << 0;
    std::cout.flush();
    unsigned char buf2[FileNode::MiniStatBinarySize];
    FileNode::SerializeStat(&buf, buf2);
    SocketListener::writeall(1, (char*)&buf2, sizeof(buf2));
}

void CreepDir(std::string path)
{
    std::forward_list<FileNode> nodes;
    auto creeper = Creeper();
    char rc = creeper.CreepPath(path, nodes);
    if (!Announcer::CreeperResult(rc, CLIAnnouncer::Log))
    {
        std::cout << rc; // alternative to writeall --- in text mode
        std::cout.flush();
        return;
    }
    std::cout << rc;
    std::cout.flush();

    //start sending node data
    unsigned char buf[FileNode::MaxBinarySize];
    std::size_t nnodes = creeper.GetResultsCount();
    LOG(INFO) << "Writing " << nnodes << " nodes.";
    SocketListener::writeall(1, (char*)&nnodes, sizeof(nnodes));
    for (auto& node: nodes)
    {
        unsigned short size = node.Serialize(buf);
        SocketListener::writeall(1, (char*)buf, size);
    }
    LOG(INFO) << "Done.";
}

int GetHomePath()
{
    auto home = Utils::GetHomePath();
    unsigned short len = home.length();

    SocketListener::writeall(1, (char*)&len, sizeof(len));

    std::cout << home;
    std::cout.flush();

    return 0;
}

int BlockDir(std::string path)
{
    LOG(INFO) << "Blocking directory " << path;
    auto blocker = Blocker();
    bool blocked = blocker.Block(path);
    SocketListener::writeall(1, (char*)&blocked, sizeof(blocked));
    return 0;
}

int UnblockDir(std::string path)
{
    LOG(INFO) << "Unblocking directory " << path;
    auto blocker = Blocker();
    bool unblocked = blocker.Unblock(path);
    SocketListener::writeall(1, (char*)&unblocked, sizeof(unblocked));
    return 0;
}

int Serve()
{
    char buff[32];
    bool isDone = false;
    char mode = 0;

    fd_set rfds;
    struct timeval tv;
    int retval;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    std::cout << 0;
    std::cout.flush();

    while (!isDone)
    {
        tv = {60, 0};
        retval = select(1, &rfds, NULL, NULL, &tv);

        if (retval == -1)
        {
            break;
        }
        else if (retval)
        {
            read(0, &mode, sizeof(mode));
            switch (mode)
            {
            case 's':
                break;
            
            default:
                break;
            }
        }
        else
        {
            mode = 0;
        }

        if (mode == 0)
            isDone = true;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    ParseArgs(argc, argv);

    if (!General::InitEverything("synccli.log"))
        return -1;

    if (GlobalCLI::mode & GlobalCLI::CLIMode::HomePath)
        GetHomePath();

    if (GlobalCLI::pathToStat != "")
        StatPath(GlobalCLI::pathToStat);

    if (GlobalCLI::dirToBlock != "")
        BlockDir(GlobalCLI::dirToBlock);

    if (GlobalCLI::dirToUnblock != "")
        UnblockDir(GlobalCLI::dirToUnblock);

    if (GlobalCLI::dirToCreep != "")
        CreepDir(GlobalCLI::dirToCreep);

    if (GlobalCLI::mode & GlobalCLI::CLIMode::DaemonServant)
    {
        Serve();
        return 0;
    }

    return 0;
}
