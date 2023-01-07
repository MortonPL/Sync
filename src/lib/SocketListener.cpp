#include "Lib/SocketListener.h"

#include <unistd.h>

#include "Utils.h"

int SocketListener::readall(int fd, char* buffer, unsigned buffer_size)
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

bool SocketListener::writeall(int fd, const char *buffer, unsigned buffer_size)
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
