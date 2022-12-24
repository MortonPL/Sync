#pragma once
#include <string>
#include <sys/types.h>
#include "xxhash.h"

#define STATUS_NEW 0
#define STATUS_DELETED 1
#define STATUS_CLEAN 2
#define STATUS_DIRTY 3
#define STATUS_MOVED 4
#define STATUS_OLD 5

/*A domain class representing a single file.*/
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
            return (one.dev < other.dev || (one.dev == other.dev && one.inode < other.inode));
        }
    };

    static const std::string StatusString[6];

    std::string path;
    dev_t dev;
    ino_t inode;
    time_t mtime;
    off_t size;
    XXH64_hash_t hashHigh;
    XXH64_hash_t hashLow;
    char status;

    devinode GetDevInode() const;
    bool IsEqualHash(const FileNode& other) const;
};
