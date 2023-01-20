#include "Lib/General.h"

#include <filesystem>
#include "Lib/DBConnector.h"
#include "Utils.h"

bool ensureDirectory(std::string dir, std::string errMsg, bool isLogger=false)
{
    try
    {
        std::filesystem::create_directories(dir);
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << errMsg << " Reason: " << e.what() << '\n';
        if (!isLogger)
            LOG(ERROR) << errMsg << " Reason: " << e.what();
        return false;
    }
}

void SetUpLogger(std::string logPath)
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");
    defaultConf.setGlobally(el::ConfigurationType::Filename, logPath);
    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
    el::Loggers::reconfigureLogger("default", defaultConf);
}

bool General::InitEverything(std::string logName)
{
    Utils::FindHomePath();

    if (!ensureDirectory(Utils::GetLogsPath(), "Failed to make sure that the logging directory exists! Exiting.", true))
        return false;
    SetUpLogger(Utils::GetLogsPath() + logName);
    LOG(INFO) << "Starting Sync.";

    if (!ensureDirectory(Utils::GetDatabasePath(), "Failed to make sure that the database directory exists! Exiting."))
        return false;
    if (!DBConnectorStatic::EnsureCreatedMain())
    {
        std::cerr << "Failed to ensure that the application database exists! Exiting.\n";
        LOG(ERROR) << "Failed to ensure that the application database exists! Exiting.";
        return false;
    }

    if (!ensureDirectory(Utils::GetTempPath(), "Failed to make sure that the temporary directory exists! Exiting."))
        return false;

    return true;
}
