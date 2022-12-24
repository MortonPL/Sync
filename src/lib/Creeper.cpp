#include "Lib/Creeper.h"

#include <fstream>
#include <filesystem>
#include "xxhash.h"
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
    ssize_t size;
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
    close(fd);
    if (size == -1)
    {
        LOG(ERROR) << "Failed to read file '" << path << "'. Error code: " << errno << ".";
        return false;
    }
    *pOut = XXH3_128bits_digest(pState);
    return true;
}

bool Creeper::CreepPath(std::string rootPath)
{
    fileNodes.clear();
    mapPath.clear();
    mapInode.clear();
    Creeper::SearchForLists(rootPath);
    auto pBuffer = malloc(BUFFER_SIZE);
    auto pState = XXH3_createState();

    for (auto const& entry: std::filesystem::recursive_directory_iterator(
        rootPath, std::filesystem::directory_options::follow_directory_symlink
            | std::filesystem::directory_options::skip_permission_denied))
    {
        if (!entry.is_regular_file())
            continue;

        auto path = entry.path().string();
        std::string filePath = path.substr(rootPath.length());

        if (Creeper::CheckIfFileIsIgnored(filePath))
            continue;

        //stat the file
        struct stat buf;
        auto cstr = path.c_str();
        stat(cstr, &buf);

        // calculate hash
        XXH128_hash_t hash;
        if (!hashFile(pState, pBuffer, path, cstr, &hash))
        {
            free(pBuffer);
            XXH3_freeState(pState);
            return false;
        }

        // create a new file entry
        FileNode node(filePath, buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size, hash.high64, hash.low64);
        fileNodes.push_back(node);
        FileNode* pNode = &fileNodes.back();
        mapPath.emplace(filePath, pNode);
        mapInode.emplace(node.GetDevInode(), pNode);
    }
    XXH3_freeState(pState);
    free(pBuffer);

    return true;
}
#undef BUFFER_SIZE
#undef GUARD_FAIL

std::list<FileNode>* Creeper::GetResults()
{
    return &fileNodes;
}
