#pragma once
#include <string>
#include <netinet/in.h>

/*A wrapper class for managing TCP communications.*/
class SocketListener
{
public:
    static int readall(int fd, char* buffer, unsigned buffer_size);
    static bool writeall(int fd, const char *buffer, unsigned buffer_size);
};
