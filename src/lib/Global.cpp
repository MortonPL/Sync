#include "Lib/Global.h"

Configuration Global::config;
bool Global::hasLoadedConfig = false;
SSHConnector Global::ssh;
bool Global::hasEstablishedConnection = false;

bool Global::IsLoadedConfig()
{
    return Global::hasLoadedConfig;
}

const Configuration& Global::GetCurrentConfig()
{
    return Global::config;
}

void Global::SetCurrentConfig(const Configuration& config)
{
    Global::hasLoadedConfig = true;
    Global::config = config;
}

bool Global::IsEstablishedConnection()
{
    return Global::hasEstablishedConnection;
}

const SSHConnector& Global::GetConnection()
{
    return Global::ssh;
}

void Global::SetConnection()
{
    if (Global::hasEstablishedConnection)
    {
        Global::ssh.EndSession();
    }
    Global::hasEstablishedConnection = false;
}

void Global::SetConnection(const SSHConnector& ssh)
{
    if (Global::hasEstablishedConnection)
    {
        Global::ssh.EndSession();
    }
    Global::hasEstablishedConnection = true;
    Global::ssh = ssh;
}
