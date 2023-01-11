#pragma once
#include <string>
#include <netinet/in.h>

/*Previously a wrapper class for managing TCP communications.*/
//TODO Properly rename
class SocketListener
{
public:
    static int readall(int fd, char* buffer, unsigned buffer_size);
    static bool writeall(int fd, const char *buffer, unsigned buffer_size);
};
