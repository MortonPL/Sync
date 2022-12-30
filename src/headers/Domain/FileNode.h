#pragma once
#include <string>
#include <sys/types.h>
#include <map>
#include "xxhash.h"

/*A domain class representing a single file.*/
class FileNode
{
public:
    FileNode();
    FileNode(std::string path);
    FileNode(std::string path, dev_t dev, ino_t inode, time_t mtime, off_t size,
             XXH64_hash_t hashHigh, XXH64_hash_t hashLow);
    ~FileNode();

    enum Status: char
    {
        New = 0,
        Deleted,
        Clean,
        Dirty,
        Moved, //???
        Absent,
        HistoryPresent,
    };

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

    static const unsigned short MaxBinarySize;
    static const std::map<Status, std::string> StatusAsString;

    std::string path;
    dev_t dev;
    ino_t inode;
    time_t mtime = 0;
    off_t size = 0;
    XXH64_hash_t hashHigh;
    XXH64_hash_t hashLow;
    Status status = Status::New;

    devinode GetDevInode() const;
    bool IsEqualHash(const FileNode& other) const;
    unsigned short Serialize(unsigned char* buf) const;
    static FileNode Deserialize(unsigned char* buf);
};
