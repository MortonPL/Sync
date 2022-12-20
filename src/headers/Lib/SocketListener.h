#pragma once
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

class SocketListener
{
public:
    SocketListener();
    ~SocketListener();

    bool Init(std::string address, int port, bool isServer=true);
    bool DeInit();
    bool Bind();
    bool Listen();
    bool Accept();
    bool EndAccept();
    bool Connect();
    int Read(char* buffer, int size);
    int Write(const char* buffer, int size);

private:
    int err = 0;
    int ourFd = -1;
    int theirFd = -1;
    struct sockaddr_in6 ourAddress;
    struct sockaddr_in6 theirAddress;
    
};
