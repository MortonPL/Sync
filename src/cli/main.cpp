#include <sys/stat.h>
#include <unistd.h>

#include "Lib/General.h"
#include "Lib/Creeper.h"
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
        case 'p':
            if (i + 1 >= argc)
                break;
            GlobalCLI::rootDir = Utils::CorrectDirPath(argv[i+1]);
            break;
        case 'r':
            if (i + 2 >= argc)
                break;
            GlobalCLI::remoteAddress = argv[i+1];
            GlobalCLI::remotePort = atoi(argv[i+2]);
            break;
        case 'h':
            GlobalCLI::mode |= GlobalCLI::CLIMode::HomePath;
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
    
    if (GlobalCLI::rootDir != "")
        std::filesystem::current_path(GlobalCLI::rootDir);

    if (GlobalCLI::mode & GlobalCLI::CLIMode::HomePath)
    {
        GetHomePath();
    }

    if (GlobalCLI::dirToCreep != "")
    {
        CreepDir(GlobalCLI::dirToCreep);
    }

    if (GlobalCLI::mode & GlobalCLI::CLIMode::DaemonServant)
    {
        Serve();
        return 0;
    }

    return 0;
}
