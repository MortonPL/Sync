#include "Lib/Creeper.h"

#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "Utils.h"

std::string Creeper::SyncBlackListFile = ".SyncBlackList";
std::string Creeper::SyncWhiteListFile = ".SyncWhiteList";

Creeper::HasherState::HasherState()
{
    pState = std::unique_ptr<XXH3_state_t, hasherFree>(XXH3_createState(), hasherFree());
    buffer = std::vector<char>(bufferSize, 0);
}

Creeper::HasherState::~HasherState()
{
}

Creeper::Creeper()
{
}

Creeper::~Creeper()
{
}

void Creeper::SearchForLists(const std::string& path)
{
    auto readList = [](const std::string& path, const std::string& filename, std::list<std::regex>& rules)
    {
        std::ifstream in(path + filename);
        if (in.is_open())
        {
            rules.clear();
            std::string out;
            while (getline(in, out))
            {
                if(*out.end() == '\r')
                    out.pop_back();
                Utils::Replace(out, ".", "\\.");
                Utils::Replace(out, "*", ".*");
                try
                {
                    rules.push_back(std::regex(out));
                }
                catch(const std::exception& e)
                {
                    LOG(ERROR) << fmt::format("Failed to parse the following {} regex rule: {}", filename, out);
                }
            }
        }
    };

    whitelist.clear();
    blacklist.clear();
    readList(path, Creeper::SyncWhiteListFile, whitelist);
    readList(path, Creeper::SyncBlackListFile, blacklist);
}

bool Creeper::CheckIfFileIsIgnored(const std::string& path) const
{
    for (auto const& regex: whitelist)
        if (std::regex_match(path, regex))
            return false;

    for (auto const& regex: blacklist)
        if (std::regex_match(path, regex))
            return true;

    return false;
}

XXH128_hash_t Creeper::HashFile(HasherState& state, const std::string& path)
{
    if(XXH3_128bits_reset(state.pState.get()) == XXH_ERROR)
        throw std::runtime_error("Failed to reset hashing machine while processing file " + path);

    XXH128_hash_t result;
    try
    {
        std::ifstream inputStream(path, std::ios::binary);
        auto pData = state.buffer.data();
        inputStream.read(pData, state.bufferSize);
        size_t len;
        // NOTE: reading less than bufferSize bytes sets failbit, so we can't put read() as the condiiton!
        while((len = inputStream? state.bufferSize: inputStream.gcount()) > 0)
        {
            if(XXH3_128bits_update(state.pState.get(), pData, len) == XXH_ERROR)
                throw std::runtime_error("Unspecified hasher failure.");
            inputStream.read(pData, state.bufferSize);
        }
        result = XXH3_128bits_digest(state.pState.get());
    }
    catch(const std::exception& e)
    {
        throw std::runtime_error("Failed to hash file " + path + " Reason: " + e.what());
    }

    return result;
}

Creeper::Result Creeper::CheckIfPathExists(const std::string& path, struct stat& buf, const bool isDir)
{
    int rc = stat(path.c_str(), &buf);
    int err = errno;
    if (rc == -1)
    {
        if (err == EACCES)
            return Result::Permissions;
        if (err == ENOENT || err == ENOTDIR)
            return Result::NotExists;
        LOG(ERROR) << "An error occured while checking if a path exists. Error code: " << err;
        return Result::Error;
    }
    if (isDir)
    {
        if (!S_ISDIR(buf.st_mode))
            return Result::NotADir;
        if ((buf.st_mode & S_IRWXU) != S_IRWXU)
            return Result::Permissions;
    }
    else
    {
        if (!S_ISREG(buf.st_mode))
            return Result::NotADir;
        if ((buf.st_mode & (S_IREAD|S_IWRITE)) != (S_IREAD|S_IWRITE))
            return Result::Permissions;
    }
    
    return Result::Ok;
}

bool Creeper::MakeNode(const std::filesystem::directory_entry& entry,
                       const std::string& rootPath, HasherState& state, FileNode& node) const
{
    if (!entry.is_regular_file())
        return false;

    std::string path = entry.path().string();
    std::string filePath = path.substr(rootPath.length());

    if (CheckIfFileIsIgnored(filePath))
        return false;

    struct stat buf;
    stat(path.c_str(), &buf);

    XXH128_hash_t hash = HashFile(state, path);

    node = FileNode(filePath, buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size, hash.high64, hash.low64);
    return true;
}

Creeper::Result Creeper::MakeSingleNode(const std::string& path, FileNode& node)
{
    Result result;
    struct stat buf;
    if ((result = CheckIfPathExists(path, buf, false)) != Result::Ok)
        return result;

    HasherState state;
    XXH128_hash_t hash;
    try
    {
        hash = HashFile(state, path);
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to hash file " + path + " Reason: " + e.what();
        return Result::Error;
    }

    node = FileNode(path, buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size, hash.high64, hash.low64);
    return Result::Ok;
}

Creeper::Result Creeper::MakeSingleNodeLight(const std::string& path, FileNode& node)
{
    Result result;
    struct stat buf;
    if ((result = CheckIfPathExists(path, buf, false)) != Result::Ok)
        return result;

    node = FileNode(path, buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size, 0, 0);
    node.noHash = true;
    return Result::Ok;
}

Creeper::Result Creeper::CreepPath(std::string rootPath, std::forward_list<FileNode>& fileNodes)
{
    Result result;
    struct stat buf;
    if ((result = CheckIfPathExists(rootPath, buf, true)) != Result::Ok)
        return result;

    SearchForLists(rootPath);
    HasherState state;

    try
    {
        for (auto const& entry: std::filesystem::recursive_directory_iterator(
            rootPath, std::filesystem::directory_options::skip_permission_denied))
        {
            FileNode node;
            if (!MakeNode(entry, rootPath, state, node))
                continue;

            fileNodes.push_front(node);
            fileNodesCount++;
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "An error has occured while scanning for changes: " << e.what();
        return Result::Error;
    }

    return Result::Ok;
}

size_t Creeper::GetResultsCount() const
{
    return fileNodesCount;
}
