#pragma once
#include <string>
#include <sys/types.h>

class FileNode
{
public:
    FileNode(std::string path, dev_t dev, ino_t inode, time_t mtime, off_t size);
    FileNode(unsigned char hashpath[], dev_t dev, ino_t inode, time_t mtime, off_t size);
    ~FileNode();

    std::string path;
    dev_t dev;
    ino_t inode;
    time_t mtime;
    off_t size;
    char status;
};
