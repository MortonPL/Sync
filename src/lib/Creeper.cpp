#include "Lib/Creeper.h"

#include <fstream>
#include <fcntl.h>
#include <unistd.h>

#include "Utils.h"

std::list<FileNode> Creeper::fileNodes;
std::vector<std::regex> Creeper::whitelist;
std::vector<std::regex> Creeper::blacklist;
std::map<std::string, FileNode*> Creeper::mapPath;
std::map<FileNode::devinode, FileNode*> Creeper::mapInode;

void Creeper::SearchForLists(std::string path)
{
    auto readList = [](std::string path, std::string filename, std::vector<std::regex>& rules)
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
    readList(path, ".SyncWhitelist", whitelist);
    readList(path, ".SyncBlacklist", blacklist);
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
        LOG(ERROR) << "Failed to reset hashing machine while processing file '" << path << "'.";\
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

void Creeper::PreCreepCleanup(std::string rootPath, XXH3_state_t*& pState, void*& pBuffer)
{
    fileNodes.clear();
    mapPath.clear();
    mapInode.clear();
    Creeper::SearchForLists(rootPath);
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

    if (Creeper::CheckIfFileIsIgnored(filePath))
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
    node = FileNode(filePath, "", buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size, hash.high64, hash.low64);
    return RES_OK;
}

bool Creeper::CreepPathNoMap(std::string rootPath)
{
    XXH3_state_t* pState;
    void* pBuffer;
    Creeper::PreCreepCleanup(rootPath, pState, pBuffer);

    for (auto const& entry: std::filesystem::recursive_directory_iterator(
        rootPath, std::filesystem::directory_options::follow_directory_symlink
            | std::filesystem::directory_options::skip_permission_denied))
    {
        FileNode node;      
        switch(Creeper::MakeNode(entry, rootPath, pState, pBuffer, node))
        {
        case RES_CONTINUE:
            continue;
        default:
        case RES_ERROR:
            free(pBuffer);
            XXH3_freeState(pState);
            return false;
        case RES_OK:
            break;
        }

        fileNodes.push_back(node);
    }
    XXH3_freeState(pState);
    free(pBuffer);

    return true;
}

bool Creeper::CreepPath(std::string rootPath)
{
    XXH3_state_t* pState;
    void* pBuffer;
    Creeper::PreCreepCleanup(rootPath, pState, pBuffer);

    for (auto const& entry: std::filesystem::recursive_directory_iterator(
        rootPath, std::filesystem::directory_options::follow_directory_symlink
            | std::filesystem::directory_options::skip_permission_denied))
    {
        FileNode node;      
        switch(Creeper::MakeNode(entry, rootPath, pState, pBuffer, node))
        {
        case RES_CONTINUE:
            continue;
        default:
        case RES_ERROR:
            free(pBuffer);
            XXH3_freeState(pState);
            return false;
        case RES_OK:
            break;
        }

        fileNodes.push_back(node);
        FileNode* pNodeMapPtr = &fileNodes.back();
        mapPath.emplace(node.path, pNodeMapPtr);
        mapInode.emplace(node.GetDevInode(), pNodeMapPtr);
    }
    XXH3_freeState(pState);
    free(pBuffer);

    return true;
}
#undef BUFFER_SIZE
#undef RES_ERROR
#undef RES_CONTINUE
#undef RES_OK

std::list<FileNode>* Creeper::GetResults()
{
    return &fileNodes;
}

FileNode* Creeper::FindMapPath(std::string path)
{
    auto it = Creeper::mapPath.find(path);
    if (it == Creeper::mapPath.end())
        return nullptr;
    return it->second;
}

FileNode* Creeper::FindMapInode(FileNode::devinode inode)
{
    auto it = Creeper::mapInode.find(inode);
    if (it == Creeper::mapInode.end())
        return nullptr;
    return it->second;
}
