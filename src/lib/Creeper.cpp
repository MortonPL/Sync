#include "Lib/Creeper.h"

#include <fstream>
#include <filesystem>
#include "xxhash.h"
#include <fcntl.h>
#include <unistd.h>

#include "Utils.h"

std::string Creeper::path;
std::list<FileNode> Creeper::fileNodes;
std::vector<std::regex> Creeper::whitelist;
std::vector<std::regex> Creeper::blacklist;
std::map<std::string, FileNode*> Creeper::mapPath;
std::map<FileNode::devinode, FileNode*> Creeper::mapInode;

Creeper::Creeper(std::string path)
{
    this->path = path;
}

Creeper::~Creeper()
{
}

void Creeper::SearchForLists(std::string path)
{
    auto readList = [](std::string path, std::string filename, std::vector<std::regex>& rules) {
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
                    std::regex rule(out);
                    rules.push_back(rule);
                }
                catch(const std::exception& e)
                {
                    LOG(ERROR) << fmt::format("Failed to parse the following {} regex rule: ", filename, out);
                }
            }
        }
    };

    whitelist.clear();
    blacklist.clear();
    readList(path, ".SyncWhitelist", whitelist);
    readList(path, ".SyncBlacklist", blacklist);
}

bool Creeper::CheckBlackWhiteLists(std::string path)
{
    for (auto const& regex: whitelist)
        if (std::regex_search(path, regex))
            return true;

    for (auto const& regex: blacklist)
        if (std::regex_search(path, regex))
            return false;

    return true;
}

#define GUARD_FAIL(pred)\
if (pred)\
{\
    LOG(ERROR) << "Failed to hash file: " << str;\
    free(buffer);\
    return false;\
}
#define BUFFER_SIZE 4096

bool Creeper::CreepPath(std::string path)
{
    fileNodes.clear();
    mapPath.clear();
    mapInode.clear();
    Creeper::SearchForLists(path);
    auto buffer = malloc(BUFFER_SIZE);
    auto state = XXH3_createState();

    for (auto const& entry: std::filesystem::recursive_directory_iterator(
        path, std::filesystem::directory_options::follow_directory_symlink
            | std::filesystem::directory_options::skip_permission_denied))
    {
        if (!entry.is_regular_file())
            continue;

        auto str = entry.path().string();
        std::string filePath = str.substr(path.length());

        if (!Creeper::CheckBlackWhiteLists(filePath))
            continue;

        //stat the file
        struct stat buf;
        auto cstr = str.c_str();
        stat(cstr, &buf);

        // calculate hash
        GUARD_FAIL(XXH3_128bits_reset(state) == XXH_ERROR);
        ssize_t size;
        auto fd = open(cstr, O_RDONLY);
        while((size = read(fd, buffer, BUFFER_SIZE)) > 0)
        {
            GUARD_FAIL(XXH3_128bits_update(state, buffer, size) == XXH_ERROR);
        }
        auto hash = XXH3_128bits_digest(state);

        // make node
        FileNode node(filePath, buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size, hash.high64, hash.low64);
        fileNodes.push_back(node);
        FileNode* pNode = &fileNodes.back();
        mapPath.emplace(filePath, pNode);
        mapInode.emplace(pNode->GetDevInode(), pNode);
    }

    return true;
}

#undef BUFFER_SIZE
#undef FAIL

std::list<FileNode>* Creeper::GetResults()
{
    return &fileNodes;
}
