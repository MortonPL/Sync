#ifndef SRC_DOMAIN_FILE_NODE_H
#define SRC_DOMAIN_FILE_NODE_H
#include <string>
#include <vector>
#include <sys/stat.h>
#include <map>
#include "xxhash.h"

/*A domain class representing a single file.*/
class FileNode
{
public:
    FileNode();
    FileNode(const std::string path);
    FileNode(const std::string path, const dev_t dev, const ino_t inode, const time_t mtime, const off_t size,
             const XXH64_hash_t hashHigh, const XXH64_hash_t hashLow);
    ~FileNode();

    enum class Status: char
    {
        None = 0,
        New,
        Clean,
        Dirty,
        Changed,
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

    bool operator<(const FileNode& other) const
    {
        return path < other.path;
    }

    static const std::size_t MiniStatBinarySize;
    static const std::map<Status, std::string> StatusAsString;
    static const std::size_t minimumNodeBinarySize;

    typedef std::byte MarshallingUnit;
    typedef std::vector<MarshallingUnit> MarshallingContainer;

    std::string path;
    dev_t dev;
    ino_t inode;
    time_t mtime = 0;
    off_t size = 0;
    XXH64_hash_t hashHigh;
    XXH64_hash_t hashLow;
    Status status = Status::Absent;
    bool noHash = true;

    devinode GetDevInode() const;
    bool IsEqualHash(const FileNode& other) const;
    std::string HashToString() const;
    bool IsEmpty() const;

    std::size_t Serialize(MarshallingContainer& buf) const;
    static std::size_t Serialize(MarshallingContainer& buf, const FileNode& node);
    static FileNode Deserialize(MarshallingContainer& buf);
    static void SerializeStat(struct stat* in, MarshallingContainer& out);
    static void DeserializeStat(MarshallingContainer& in, struct stat* out);
};

#endif
