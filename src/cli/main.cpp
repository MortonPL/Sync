#include <sys/stat.h>
#include <sys/types.h>

#include "Lib/General.h"
#include "Lib/Creeper.h"
#include "Lib/DBConnector.h"
#include "Lib/SSHConnector.h"
#include "Lib/SocketListener.h"
#include "CLI/Global.h"
#include "CLI/MainLoop.h"
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
        case 'D':
            GlobalCLI::mode |= GlobalCLI::CLIMode::DaemonMaster;
            break;
        case 'r':
            if (i + 2 >= argc)
                break;
            GlobalCLI::remoteAddress = argv[i+1];
            GlobalCLI::remotePort = atoi(argv[i+2]);
            break;
        default:
            break;
        }
    }
}

void CreepDir(std::string path)
{
    char rc = Creeper::CreepPathNoMap(path);
    if(rc != CREEP_OK)
    {
        LOG(ERROR) << "Failed to scan for files in the given directory.";
        std::cout << rc; // alternative to writeall --- in text mode
        std::cout.flush();
        return;
    }
    std::cout << rc;
    std::cout.flush();
    auto nodes = Creeper::GetResults();

    unsigned char buf[FileNode::MaxBinarySize];
    std::size_t nnodes = nodes->size();
    LOG(INFO) << "Writing " << nnodes << "nodes.";
    SocketListener::writeall(1, (char*)&nnodes, sizeof(nnodes));
    for (auto& node: *nodes)
    {
        unsigned short size = node.Serialize(buf);
        SocketListener::writeall(1, (char*)buf, size);
    }
    LOG(INFO) << "Done.";
}

int Serve()
{
    auto s = SocketListener();
    if (!s.Init(GlobalCLI::remoteAddress, GlobalCLI::remotePort))
    {
        s.DeInit();
        LOG(ERROR) << "Failed to initialize socket";
    }
    if (!s.Bind())
    {
        s.DeInit();
        LOG(ERROR) << "Failed to bind socket";
    }
    if (!s.Listen())
    {
        s.DeInit();
        LOG(ERROR) << "Failed to listen to the socket";
    }

    char buff[32];
    bool isDone = false;
    while (!isDone)
    {
        LOG(INFO) << "Loop";
        if (!s.Accept())
        {
            s.DeInit();
            LOG(ERROR) << "Failed to accept a connection";
            continue;
        }
        if (s.Read(buff, 32) == -1)
        {
            s.DeInit();
            LOG(ERROR) << "Error receiving data";
            s.EndAccept();
            return 4;
        }

        s.EndAccept();
        isDone = true;
    }
    s.DeInit();
    return 0;
}

int Master()
{
    return 0;
}

int main(int argc, char* argv[])
{
    ParseArgs(argc, argv);

    if (!General::InitEverything("synccli.log"))
        return -1;

    if (GlobalCLI::dirToCreep != "")
    {
        CreepDir(GlobalCLI::dirToCreep);
    }

    if (GlobalCLI::mode & GlobalCLI::CLIMode::DaemonServant)
    {
        if (fork() == 0) // parent kills itself here, child becomes a daemon
            Serve();
        return 0;
    }

    if (GlobalCLI::mode & GlobalCLI::CLIMode::DaemonMaster)
    {
        if (fork() == 0) // parent kills itself here, child becomes a daemon
            Master();
        return 0;
    }

    return 0;
}
