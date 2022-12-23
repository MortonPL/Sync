#pragma once
#include <string>
#include <sys/types.h>
#include "xxhash.h"

class FileNode
{
public:
    FileNode(std::string path, dev_t dev, ino_t inode, time_t mtime, off_t size, XXH64_hash_t hashHigh, XXH64_hash_t hashLow);
    ~FileNode();

    struct devinode
    {
        dev_t dev;
        ino_t inode;

        friend bool operator <(const devinode& one, const devinode& other)
        {
            return (one.dev < other.dev) && (one.inode < other.inode);
        }
    };

    devinode GetDevInode();

    std::string path;
    dev_t dev;
    ino_t inode;
    time_t mtime;
    off_t size;
    XXH64_hash_t hashHigh;
    XXH64_hash_t hashLow;
    char status;
};
