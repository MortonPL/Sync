#pragma once
#include <string>
#include <sys/types.h>
#include "xxhash.h"

#define STATUS_NEW 0
#define STATUS_DELETED 1
#define STATUS_CLEAN 2
#define STATUS_DIRTY 3
#define STATUS_MOVED 4
#define STATUS_HISTORY_PRESENT 5

/*A domain class representing a single file.*/
class FileNode
{
public:
    FileNode();
    FileNode(std::string path);
    FileNode(std::string path, dev_t dev, ino_t inode, time_t mtime, off_t size,
             XXH64_hash_t hashHigh, XXH64_hash_t hashLow);
    ~FileNode();

    struct devinode
    {
        dev_t dev;
        ino_t inode;

        bool operator==(const devinode& other) const
        {
            return dev == other.dev && inode == other.inode;
        }

        struct devinodeHasher
        {
            std::size_t operator()(const devinode& devinode) const
            {
                return std::hash<dev_t>()(devinode.dev) ^ (std::hash<ino_t>()(devinode.inode) << 1);
            }
        };
    };

    static const std::string StatusString[6];
    static const unsigned short MaxBinarySize;

    std::string path;
    dev_t dev;
    ino_t inode;
    time_t mtime = 0;
    off_t size = 0;
    XXH64_hash_t hashHigh;
    XXH64_hash_t hashLow;
    char status = STATUS_NEW;

    devinode GetDevInode() const;
    bool IsEqualHash(const FileNode& other) const;
    unsigned short Serialize(unsigned char* buf) const;
    static FileNode Deserialize(unsigned char* buf);
};
