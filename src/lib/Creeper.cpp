#include "Lib/Creeper.h"

#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "Utils.h"

std::string Creeper::SyncBlackListFile = ".SyncBlackList";
std::string Creeper::SyncWhiteListFile = ".SyncWhiteList";
std::string Creeper::SyncBlockedFile = ".SyncBLOCKED";

Creeper::Creeper()
{
}

Creeper::~Creeper()
{
}

void Creeper::SearchForLists(std::string& path)
{
    auto readList = [](std::string& path, std::string filename, std::list<std::regex>& rules)
    {
        std::ifstream in(path + filename, std::ios_base::in);
        if (in.is_open())
        {
            rules.clear();
            std::string out;
            while (in.peek() != EOF)
            {
                getline(in, out);
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

bool Creeper::CheckIfFileIsIgnored(std::string path)
{
    for (auto const& regex: whitelist)
        if (std::regex_search(path, regex))
            return false;

    for (auto const& regex: blacklist)
        if (std::regex_search(path, regex))
            return true;

    return false;
}

#define BUFFER_SIZE 4096
bool hashFile(XXH3_state_t* pState, void* pBuffer, std::string& path, const char* cPath, XXH128_hash_t* pOut)
{
    if(XXH3_128bits_reset(pState) == XXH_ERROR)
    {
        LOG(ERROR) << "Failed to reset hashing machine while processing file '" << path << "'.";
        return false;
    }
    ssize_t size = 0;
    int fd = -1;
    if ((fd = open(cPath, O_RDONLY)) == -1)
    {
        LOG(ERROR) << "Failed to open file '" << path << "'. Error code: " << errno << ".";
        return false;
    }
    while((size = read(fd, pBuffer, BUFFER_SIZE)) > 0)
    {
        if(XXH3_128bits_update(pState, pBuffer, size) == XXH_ERROR)
        {
            LOG(ERROR) << "Failed to hash file '" << path << "'.";
            close(fd);
            return false;
        }
    }
    auto e = errno;
    close(fd);
    if (size == -1)
    {
        LOG(ERROR) << "Failed to read file '" << path << "'. ERRNO: " << e << ".";
        return false;
    }
    *pOut = XXH3_128bits_digest(pState);
    return true;
}

int Creeper::CheckIfDirExists(std::string& path)
{
    struct stat ret;
    int rc = stat(path.c_str(), &ret);
    int err = errno;
    if (rc == -1)
    {
        if (err == EACCES)
            return CREEP_PERM;
        if (err == ENOENT || err == ENOTDIR)
            return CREEP_EXIST;
        LOG(ERROR) << "An error occured while checking if a directory exists. Error code: " << err;
        return CREEP_ERROR;
    }
    if (!S_ISDIR(ret.st_mode))
        return CREEP_NOTDIR;
    if ((ret.st_mode & S_IRWXU) != S_IRWXU)
        return CREEP_PERM;
    
    return CREEP_OK;
}

void Creeper::PreCreepCleanup(std::string& rootPath, XXH3_state_t*& pState, void*& pBuffer)
{
    SearchForLists(rootPath);
    pBuffer = malloc(BUFFER_SIZE);
    pState = XXH3_createState();
}

#define RES_ERROR -1
#define RES_CONTINUE 0
#define RES_OK 2
int Creeper::MakeNode(const std::filesystem::__cxx11::directory_entry& entry,
                      std::string& rootPath, XXH3_state_t* pState, void* pBuffer, FileNode& node)
{
    if (!entry.is_regular_file())
        return RES_CONTINUE;

    std::string path = entry.path().string();
    std::string filePath = path.substr(rootPath.length());

    if (CheckIfFileIsIgnored(filePath))
        return RES_CONTINUE;

    //stat the file
    struct stat buf;
    auto cstr = path.c_str();
    stat(cstr, &buf);

    // calculate hash
    XXH128_hash_t hash;
    if (!hashFile(pState, pBuffer, path, cstr, &hash))
        return RES_ERROR;

    // create a new file entry
    node = FileNode(filePath, buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size, hash.high64, hash.low64);
    return RES_OK;
}

int Creeper::MakeNode(std::string& path, FileNode& node)
{
    auto cstr = path.c_str();
    struct stat ret;
    int rc = stat(cstr, &ret);
    int err = errno;
    if (rc == -1)
    {
        if (err == EACCES)
            return CREEP_PERM;
        if (err == ENOENT || err == ENOTDIR)
            return CREEP_EXIST;
        LOG(ERROR) << "An error occured while checking if a path exists. Error code: " << err;
        return CREEP_ERROR;
    }
    if (!S_ISREG(ret.st_mode))
        return CREEP_NOTDIR;
    if ((ret.st_mode & S_IRWXU) != S_IRWXU)
        return CREEP_PERM;

    // calculate hash
    auto pBuffer = malloc(BUFFER_SIZE);
    auto pState = XXH3_createState();
    XXH128_hash_t hash;
    if (!hashFile(pState, pBuffer, path, cstr, &hash))
    {
        free(pBuffer);
        XXH3_freeState(pState);
        return CREEP_ERROR;
    }
    free(pBuffer);
    XXH3_freeState(pState);

    // create a new file entry
    node = FileNode(path, ret.st_dev, ret.st_ino, ret.st_mtim.tv_sec, ret.st_size, hash.high64, hash.low64);
    return CREEP_OK;
}

int Creeper::CreepPath(std::string rootPath, std::forward_list<FileNode>& fileNodes)
{
    XXH3_state_t* pState;
    void* pBuffer;
    int err;
    if ((err = CheckIfDirExists(rootPath)) != CREEP_OK)
        return err;
    PreCreepCleanup(rootPath, pState, pBuffer);

    try
    {
        for (auto const& entry: std::filesystem::recursive_directory_iterator(
            rootPath, std::filesystem::directory_options::skip_permission_denied))
        /*for (auto const& entry: std::filesystem::recursive_directory_iterator(
            rootPath, std::filesystem::directory_options::follow_directory_symlink
                | std::filesystem::directory_options::skip_permission_denied))*/
        {
            if (entry.path().filename().string() == Creeper::SyncBlockedFile)
                return CREEP_BLOCK;

            FileNode node;     
            switch(MakeNode(entry, rootPath, pState, pBuffer, node))
            {
            case RES_CONTINUE:
                continue;
            default:
            case RES_ERROR:
                free(pBuffer);
                XXH3_freeState(pState);
                return CREEP_ERROR;
            case RES_OK:
                break;
            }

            fileNodes.push_front(node);
            fileNodesCount++;
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
        XXH3_freeState(pState);
        free(pBuffer);
        return CREEP_ERROR;
    }
    XXH3_freeState(pState);
    free(pBuffer);

    return CREEP_OK;
}
#undef BUFFER_SIZE
#undef RES_ERROR
#undef RES_CONTINUE
#undef RES_OK

size_t Creeper::GetResultsCount()
{
    return fileNodesCount;
}
