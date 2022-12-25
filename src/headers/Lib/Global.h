#pragma once
#include <uuid/uuid.h>

#include "Domain/Configuration.h"
#include "Domain/FileNode.h"
#include "Lib/SSHConnector.h"

/*A static class for globally accessible data.*/
class Global
{
public:
    static bool IsLoadedConfig();
    static const Configuration& GetCurrentConfig();
    static void SetCurrentConfig(const Configuration& config);

    static bool IsEstablishedConnection();
    static const SSHConnector& GetConnection();
    static void SetConnection();
    static void SetConnection(const SSHConnector& ssh);

private:
    static Configuration config;
    static bool hasLoadedConfig;
    static SSHConnector ssh;
    static bool hasEstablishedConnection;
};
