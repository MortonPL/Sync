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

FileNode::FileNode(const std::string path)
{
    this->path = path;
    this->status = FileNode::Status::New;
}

FileNode::FileNode(const std::string path, const dev_t dev, const ino_t inode, const time_t mtime, const off_t size,
                   const XXH64_hash_t hashHigh, const XXH64_hash_t hashLow)
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

template<typename T>
static inline void serialize(FileNode::MarshallingUnit* pBuf, std::size_t& i, const T& field)
{
    *(T*)(pBuf + i) = field;
    i += sizeof(field);
}

static inline void serializeString(FileNode::MarshallingUnit* pBuf, std::size_t& i, const std::string& field)
{
    memcpy(pBuf + i, field.c_str(), field.size());
    i += field.size();
}

template<typename T>
static inline void deserialize(FileNode::MarshallingUnit* pBuf, std::size_t& i, T& field)
{
    field = *reinterpret_cast<T*>(pBuf + i);
    i += sizeof(field);
}

static inline void deserializeString(FileNode::MarshallingUnit* pBuf, std::size_t& i, std::string& field, std::size_t size)
{
    field = std::string((char*)(pBuf + i), size);
    i += size;
}

std::size_t FileNode::Serialize(MarshallingContainer& buf) const
{
    std::size_t i = 0;
    const std::size_t pathSize = path.size();
    const std::size_t dataSize = sizeof(pathSize) + pathSize + minimumNodeBinarySize;
    const std::size_t totalSize = sizeof(dataSize) + dataSize;
    if (buf.size() < totalSize)
        buf.resize(totalSize);
    auto pBuffer = buf.data();

    serialize(pBuffer, i, dataSize);

    serialize(pBuffer, i, pathSize);
    serializeString(pBuffer, i, path);
    serialize(pBuffer, i, size);
    serialize(pBuffer, i, mtime);
    serialize(pBuffer, i, dev);
    serialize(pBuffer, i, inode);
    serialize(pBuffer, i, hashHigh);
    serialize(pBuffer, i, hashLow);
    return totalSize;
}

std::size_t FileNode::Serialize(MarshallingContainer& buf, const FileNode& node)
{
    return node.Serialize(buf);
}

FileNode FileNode::Deserialize(MarshallingContainer& buf)
{
    std::size_t i = 0;
    auto pBuffer = buf.data();

    std::size_t pathSize;
    std::string _path;
    off_t size;
    time_t mtime;
    dev_t dev;
    ino_t inode;
    XXH64_hash_t hashHigh;
    XXH64_hash_t hashLow;

    deserialize(pBuffer, i, pathSize);
    deserializeString(pBuffer, i, _path, pathSize);
    deserialize(pBuffer, i, size);
    deserialize(pBuffer, i, mtime);
    deserialize(pBuffer, i, dev);
    deserialize(pBuffer, i, inode);
    deserialize(pBuffer, i, hashHigh);
    deserialize(pBuffer, i, hashLow);
    return FileNode(_path, dev, inode, mtime, size, hashHigh, hashLow);
}

void FileNode::SerializeStat(struct stat* in, MarshallingContainer& out)
{
    std::size_t i = 0;
    auto pBuffer = out.data();

    serialize(pBuffer, i, in->st_dev);
    serialize(pBuffer, i, in->st_ino);
    serialize(pBuffer, i, in->st_size);
    serialize(pBuffer, i, in->st_mtim.tv_sec);
}

void FileNode::DeserializeStat(MarshallingContainer& in, struct stat* out)
{
    std::size_t i = 0;
    auto pBuffer = in.data();

    deserialize(pBuffer, i, out->st_dev);
    deserialize(pBuffer, i, out->st_ino);
    deserialize(pBuffer, i, out->st_size);
    deserialize(pBuffer, i, out->st_mtim.tv_sec);
}
