#include <sys/stat.h>
#include <sys/types.h>

#include "Lib/General.h"
#include "Lib/SSHConnector.h"
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
        case 't':
            if ( i + 3 >= argc)
                break;
            Global::mode = Global::CLIMode::Test;
            Global::remoteAddress = argv[i+1];
            Global::remoteUser = argv[i+2];
            Global::remoteRoot = argv[i+3];
            break;
        case 'd':
            if ( i + 1 >= argc)
                break;
            Global::mode = Global::CLIMode::DirCheck;
            Global::dirToCheck = argv[i+1];
        default:
            break;
        }
    }
}

int CheckDir(std::string path)
{
    struct stat ret;
    if (stat(path.c_str(), &ret) == -1)
        return 1;
    if (!S_ISDIR(ret.st_mode))
        return 2;
    return 0;
}

int main(int argc, char* argv[])
{
    ParseArgs(argc, argv);

    if (!General::InitEverything("synccli.log"))
        return -1;
    
    switch (Global::mode)
    {
    case Global::CLIMode::DirCheck:
        std::cout << CheckDir(Global::dirToCheck) << '\n';
        break;
    default:
        return 0;
    }

    while (MainLoop::DoWork())
    {
    }

    return 0;
}
