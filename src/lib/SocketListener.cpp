#include "Lib/SocketListener.h"

#include <unistd.h>
#include <netinet/in.h>

#include "Utils.h"

int readall(int fd, char* buffer, unsigned buffer_size)
{
    int bytes_left = buffer_size;
    int bytes_read = 0;
    while (bytes_left > 0)
    {
        bytes_read = read(fd, buffer + buffer_size - bytes_left, bytes_left);
        if (bytes_read == 0)
            break;
        if (bytes_read == -1)
            return -1;
        bytes_left -= bytes_read;
    }
    return buffer_size - bytes_left;
}

bool writeall(int fd, const char *buffer, unsigned buffer_size)
{
    int bytes_left = buffer_size;
    int bytes_written = 0;
    while (bytes_left > 0)
    {
        bytes_written = write(fd, buffer + buffer_size - bytes_left, bytes_left);
        bytes_left -= bytes_written;
        if (bytes_written <= 0)
            return false;
    }
    return true;
}

SocketListener::SocketListener()
{
}

SocketListener::~SocketListener()
{
}

bool SocketListener::Init(std::string address, int port, bool isServer)
{
    if ((ourFd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
        return false;

    memset(&ourAddress, 0, sizeof(ourAddress));
    ourAddress.sin6_family = AF_INET6;
    ourAddress.sin6_port = htons(port);
    ourAddress.sin6_addr = in6addr_any;

    if (isServer)
    {
        int ipv6_only_enabled = 0;
        if (setsockopt(ourFd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only_enabled, sizeof(ipv6_only_enabled)) != 0)
            return false;
    }

    return true;
}

bool SocketListener::DeInit()
{
    if (ourFd != -1)
        close(ourFd);
    if (theirFd != -1)
        close(theirFd);
    return true;
}


// For server
bool SocketListener::Bind()
{
    return bind(ourFd, (const struct sockaddr*)&ourAddress, sizeof(ourAddress)) >= 0;
}

// For server
bool SocketListener::Listen()
{
    return listen(ourFd, 5) == 0;
}

// For server
bool SocketListener::Accept()
{
    memset(&theirAddress, 0, sizeof(theirAddress));
    socklen_t socklen = sizeof(theirAddress);
    theirFd = accept(ourFd, (struct sockaddr*)&theirAddress, &socklen);
    return theirFd >= 0;
}

bool SocketListener::EndAccept()
{
    close(theirFd);
    return true;
}

// For client
bool SocketListener::Connect()
{
    return connect(ourFd, (const struct sockaddr *)&ourAddress, sizeof(ourAddress)) == 0;
}

int SocketListener::Read(char* buffer, int size)
{
    int total;
    while ((total = readall(theirFd, buffer, size)) > 0)
    {
        LOG(INFO) << total << " " << buffer;
    }
    return total;
}

int SocketListener::Write(const char* buffer, int size)
{
    return writeall(ourFd, buffer, size);
}
