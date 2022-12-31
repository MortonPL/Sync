#include "Domain/FileNode.h"

#include "linux/limits.h"
#include <cstring>

const unsigned short FileNode::MaxBinarySize = sizeof(unsigned short) + sizeof(unsigned short)
                                               + PATH_MAX + sizeof(FileNode::size) + sizeof(FileNode::mtime)
                                               + sizeof(FileNode::dev) + sizeof(FileNode::inode)
                                               + sizeof(FileNode::hashHigh) + sizeof(FileNode::hashLow);

const std::map<FileNode::Status, std::string> FileNode::StatusAsString =
{
    {FileNode::Status::None, "None"},
    {FileNode::Status::New, "New"},
    {FileNode::Status::Deleted, "Deleted"},
    {FileNode::Status::Clean, "Clean"},
    {FileNode::Status::Dirty, "Dirty"},
    {FileNode::Status::MovedClean, "Clean (Moved)"},
    {FileNode::Status::MovedDirty, "Dirty (Moved)"},
    {FileNode::Status::HistoryPresent, "Present"},
    {FileNode::Status::Absent, "Absent"},
};

FileNode::FileNode()
{
}

FileNode::FileNode(std::string path)
{
    this->path = path;
}

FileNode::FileNode(std::string path, dev_t dev, ino_t inode, time_t mtime, off_t size,
             XXH64_hash_t hashHigh, XXH64_hash_t hashLow)
{
    this->path = path;
    this->dev = dev;
    this->inode = inode;
    this->mtime = mtime;
    this->size = size;
    this->hashHigh = hashHigh;
    this->hashLow = hashLow;
}

FileNode::~FileNode()
{
}

FileNode::devinode FileNode::GetDevInode() const
{
    return devinode{dev, inode};
}

bool FileNode::IsEqualHash(const FileNode& other) const
{
    return hashHigh == other.hashHigh && hashLow == other.hashLow;
}

#define SERIALIZE(type, field)\
    *(type*)(buf + i) = field;\
    i += sizeof(type);

#define SERIALIZE_STRING(field, size)\
    memcpy(buf + i, field.c_str(), size);\
    i += size;

#define DESERIALIZE(type, field)\
    type field = *reinterpret_cast<type*>(buf + i);\
    i += sizeof(type);

#define DESERIALIZE_STRING(field, size)\
    std::string field((char*)(buf + i), size);\
    i += size;

unsigned short FileNode::Serialize(unsigned char* buf) const
{
    unsigned short i = 0;
    unsigned short pathSize = path.size();
    unsigned short dataSize = sizeof(pathSize) + pathSize + sizeof(size) + sizeof(mtime) + sizeof(dev) + sizeof(inode) + sizeof(hashHigh) + sizeof(hashLow);

    SERIALIZE(unsigned short, dataSize)

    SERIALIZE(unsigned short, pathSize)
    SERIALIZE_STRING(path, pathSize)
    SERIALIZE(off_t, size)
    SERIALIZE(time_t, mtime)
    SERIALIZE(dev_t, dev)
    SERIALIZE(ino_t, inode)
    SERIALIZE(XXH64_hash_t, hashHigh)
    SERIALIZE(XXH64_hash_t, hashLow)
    return sizeof(dataSize) + dataSize;
}

FileNode FileNode::Deserialize(unsigned char* buf)
{
    unsigned short i = 0;

    DESERIALIZE(unsigned short, pathSize)
    DESERIALIZE_STRING(_path, pathSize)
    DESERIALIZE(off_t, size)
    DESERIALIZE(time_t, mtime)
    DESERIALIZE(dev_t, dev)
    DESERIALIZE(ino_t, inode)
    DESERIALIZE(XXH64_hash_t, hashHigh)
    DESERIALIZE(XXH64_hash_t, hashLow)
    return FileNode(_path, dev, inode, mtime, size, hashHigh, hashLow);
}

#undef SERIALIZE
#undef SERIALIZE_STRING
#undef DESERIALIZE
#undef DESERIALIZE_STRING
