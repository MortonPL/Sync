#include "Domain/FileNode.h"

#include <limits.h>
#include <cstring>

#include "Utils.h"

const unsigned short FileNode::MaxBinarySize = sizeof(unsigned short) + sizeof(unsigned short)
                                               + PATH_MAX + sizeof(FileNode::size) + sizeof(FileNode::mtime)
                                               + sizeof(FileNode::dev) + sizeof(FileNode::inode)
                                               + sizeof(FileNode::hashHigh) + sizeof(FileNode::hashLow);
const unsigned short FileNode::MiniStatBinarySize = sizeof(dev_t) + sizeof(ino_t) + sizeof(off_t) + sizeof(time_t);

const std::map<FileNode::Status, std::string> FileNode::StatusAsString =
{
    {FileNode::Status::None, "None"},
    {FileNode::Status::New, "New"},
    {FileNode::Status::Clean, "Clean"},
    {FileNode::Status::Dirty, "Dirty"},
    {FileNode::Status::HistoryPresent, "Present"},
    {FileNode::Status::Absent, "Absent"},
    {FileNode::Status::Changed, "Changed!"},
};

FileNode::FileNode()
{
}

FileNode::FileNode(std::string path)
{
    this->path = path;
    this->status = FileNode::Status::New;
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
    this->status = FileNode::Status::New;
    this->noHash = false;
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

std::string FileNode::HashToString() const
{
    return noHash? "-" : fmt::format("{:x}{:x}", (unsigned long)hashHigh, (unsigned long)hashLow);
}

bool FileNode::IsEmpty() const
{
    return status == FileNode::Status::Absent;
}

#define SERIALIZE(buf, type, field)\
    *(type*)(buf + i) = field;\
    i += sizeof(type);

#define SERIALIZE_STRING(buf, field, size)\
    memcpy(buf + i, field.c_str(), size);\
    i += size;

#define DESERIALIZE(buf, type, field)\
    field = *reinterpret_cast<type*>(buf + i);\
    i += sizeof(type);

#define DESERIALIZE_DECL(buf, type, field)\
    type field = *reinterpret_cast<type*>(buf + i);\
    i += sizeof(type);

#define DESERIALIZE_STRING(buf, field, size)\
    field((char*)(buf + i), size);\
    i += size;

#define DESERIALIZE_DECL_STRING(buf, field, size)\
    std::string field((char*)(buf + i), size);\
    i += size;

unsigned short FileNode::Serialize(unsigned char* buf) const
{
    unsigned short i = 0;
    unsigned short pathSize = path.size();
    unsigned short dataSize = sizeof(pathSize) + pathSize + sizeof(size) + sizeof(mtime) + sizeof(dev) + sizeof(inode) + sizeof(hashHigh) + sizeof(hashLow);

    SERIALIZE(buf, unsigned short, dataSize);

    SERIALIZE(buf, unsigned short, pathSize);
    SERIALIZE_STRING(buf, path, pathSize);
    SERIALIZE(buf, off_t, size);
    SERIALIZE(buf, time_t, mtime);
    SERIALIZE(buf, dev_t, dev);
    SERIALIZE(buf, ino_t, inode);
    SERIALIZE(buf, XXH64_hash_t, hashHigh);
    SERIALIZE(buf, XXH64_hash_t, hashLow);
    return sizeof(dataSize) + dataSize;
}

FileNode FileNode::Deserialize(unsigned char* buf)
{
    unsigned short i = 0;

    DESERIALIZE_DECL(buf, unsigned short, pathSize);
    DESERIALIZE_DECL_STRING(buf, _path, pathSize);
    DESERIALIZE_DECL(buf, off_t, size);
    DESERIALIZE_DECL(buf, time_t, mtime);
    DESERIALIZE_DECL(buf, dev_t, dev);
    DESERIALIZE_DECL(buf, ino_t, inode);
    DESERIALIZE_DECL(buf, XXH64_hash_t, hashHigh);
    DESERIALIZE_DECL(buf, XXH64_hash_t, hashLow);
    return FileNode(_path, dev, inode, mtime, size, hashHigh, hashLow);
}

void FileNode::SerializeStat(struct stat* in, unsigned char* out)
{
    unsigned short i = 0;

    SERIALIZE(out, dev_t, in->st_dev);
    SERIALIZE(out, ino_t, in->st_ino);
    SERIALIZE(out, off_t, in->st_size);
    SERIALIZE(out, time_t, in->st_mtim.tv_sec);
}

void FileNode::DeserializeStat(unsigned char* in, struct stat* out)
{
    unsigned short i = 0;

    DESERIALIZE(in, dev_t, out->st_dev);
    DESERIALIZE(in, ino_t, out->st_ino);
    DESERIALIZE(in, off_t, out->st_size);
    DESERIALIZE(in, time_t, out->st_mtim.tv_sec);
}

#undef SERIALIZE
#undef SERIALIZE_STRING
#undef DESERIALIZE
#undef DESERIALIZE_STRING
