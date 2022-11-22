#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "Lib/DBConnector.h"
#include "CLI/Global.h"
#include "Utils.h"

INITIALIZE_EASYLOGGINGPP

void SetUpLogger(std::string logPath)
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    //defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime %level %msg");
    // To set GLOBAL configurations you may use
    defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");
    defaultConf.setGlobally(el::ConfigurationType::Filename, logPath);
    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
    el::Loggers::reconfigureLogger("default", defaultConf);
}

void ParseArgs(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] != '-')
            continue;

        switch (argv[i][1])
        {
        case 'n':
            Global::isNetworkMode = true;
            break;
        default:
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    ParseArgs(argc, argv);

    if (mkdir(Utils::GetDataPath().c_str(), S_IRWXU | S_IRWXG) == 0 || errno == EEXIST)
    {
        SetUpLogger(Utils::GetDataPath() + "synccli.log");
        LOG(INFO) << "Starting Sync GUI.";
    }
    else
    {
        std::cout << "Failed to create application directory! Exiting.\n";
        LOG(ERROR) << "Failed to create application directory! Exiting.";
        return false;
    }

    if (!DBConnector::EnsureCreated())
    {
        std::cout << "Failed to ensure that the application database exists! Exiting.\n";
        LOG(ERROR) << "Failed to ensure that the application database exists! Exiting.";
        return false;
    }

    if (Global::isNetworkMode)
    {
        std::cout << "network!\n";
    }

    return 0;
}
