#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>

#include "../headers/Logger.h"
#include "../headers/DBConnector.h"

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

int main(int argc, char** argv)
{
    char result[PATH_MAX];
    if (readlink("/proc/self/exe", result, PATH_MAX) == -1)
        return -1;
    auto path = std::string(dirname(result));
    path.append("/SyncCLI.log");
    SetUpLogger(path);
    LOG(INFO) << "Starting Sync CLI.";

    DBConnector::EnsureCreated();

    return 0;
}
