#include "Lib/DBConnector.h"
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

int main(int argc, char** argv)
{
    SetUpLogger(Utils::GetProgramPath() + "SyncCLI.log");
    LOG(INFO) << "Starting Sync CLI.";

    DBConnector::EnsureCreated();

    return 0;
}
