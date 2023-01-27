#include "Domain/FileNode.h"

#include <limits.h>
#include <cstring>

#include "Utils.h"

const std::size_t FileNode::MiniStatBinarySize = sizeof(dev_t) + sizeof(ino_t) + sizeof(off_t) + sizeof(time_t);
const std::size_t FileNode::minimumNodeBinarySize = sizeof(dev_t) + sizeof(ino_t) + sizeof(off_t) + sizeof(time_t) + sizeof(XXH64_hash_t) + sizeof(XXH64_hash_t);

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

#define SERIALIZE(buf, i, type, field)\
    *(type*)(buf + i) = field;\
    i += sizeof(type);

#define SERIALIZE_STRING(buf, i, field, size)\
    memcpy(buf + i, field.c_str(), size);\
    i += size;

#define DESERIALIZE(buf, i, type, field)\
    field = *reinterpret_cast<type*>(buf + i);\
    i += sizeof(type);

#define DESERIALIZE_DECL(buf, i, type, field)\
    type field = *reinterpret_cast<type*>(buf + i);\
    i += sizeof(type);

#define DESERIALIZE_STRING(buf, i, field, size)\
    field((char*)(buf + i), size);\
    i += size;

#define DESERIALIZE_DECL_STRING(buf, i, field, size)\
    std::string field((char*)(buf + i), size);\
    i += size;

std::size_t FileNode::Serialize(std::vector<std::byte>& buf) const
{
    std::size_t i = 0;
    std::size_t pathSize = path.size();
    std::size_t dataSize = sizeof(pathSize) + pathSize + minimumNodeBinarySize;
    if (buf.size() < dataSize)
        buf.resize(dataSize);
    auto pBuffer = buf.data();

    SERIALIZE(pBuffer, i, std::size_t, dataSize);

    SERIALIZE(pBuffer, i, std::size_t, pathSize);
    SERIALIZE_STRING(pBuffer, i, path, pathSize);
    SERIALIZE(pBuffer, i, off_t, size);
    SERIALIZE(pBuffer, i ,time_t, mtime);
    SERIALIZE(pBuffer, i, dev_t, dev);
    SERIALIZE(pBuffer, i, ino_t, inode);
    SERIALIZE(pBuffer, i, XXH64_hash_t, hashHigh);
    SERIALIZE(pBuffer, i, XXH64_hash_t, hashLow);
    return sizeof(dataSize) + dataSize;
}

FileNode FileNode::Deserialize(std::vector<std::byte>& buf)
{
    std::size_t i = 0;
    auto pBuffer = buf.data();

    DESERIALIZE_DECL(pBuffer, i, std::size_t, pathSize);

    DESERIALIZE_DECL_STRING(pBuffer, i, _path, pathSize);
    DESERIALIZE_DECL(pBuffer, i, off_t, size);
    DESERIALIZE_DECL(pBuffer, i, time_t, mtime);
    DESERIALIZE_DECL(pBuffer, i, dev_t, dev);
    DESERIALIZE_DECL(pBuffer, i, ino_t, inode);
    DESERIALIZE_DECL(pBuffer, i, XXH64_hash_t, hashHigh);
    DESERIALIZE_DECL(pBuffer, i, XXH64_hash_t, hashLow);
    return FileNode(_path, dev, inode, mtime, size, hashHigh, hashLow);
}

void FileNode::SerializeStat(struct stat* in, MarshallingContainer& out)
{
    std::size_t i = 0;
    auto pBuffer = out.data();

    SERIALIZE(pBuffer, i, dev_t, in->st_dev);
    SERIALIZE(pBuffer, i, ino_t, in->st_ino);
    SERIALIZE(pBuffer, i, off_t, in->st_size);
    SERIALIZE(pBuffer, i, time_t, in->st_mtim.tv_sec);
}

void FileNode::DeserializeStat(MarshallingContainer& in, struct stat* out)
{
    std::size_t i = 0;
    auto pBuffer = in.data();

    DESERIALIZE(pBuffer, i, dev_t, out->st_dev);
    DESERIALIZE(pBuffer, i, ino_t, out->st_ino);
    DESERIALIZE(pBuffer, i, off_t, out->st_size);
    DESERIALIZE(pBuffer, i, time_t, out->st_mtim.tv_sec);
}

#undef SERIALIZE
#undef SERIALIZE_STRING
#undef DESERIALIZE
#undef DESERIALIZE_STRING
