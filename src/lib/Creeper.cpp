#include "Lib/Creeper.h"

#include <fstream>
#include <filesystem>
#include "Utils.h"

std::string Creeper::path;
std::vector<FileNode> Creeper::fileNodes;
std::vector<std::regex> Creeper::whitelist;
std::vector<std::regex> Creeper::blacklist;

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

void Creeper::CreepPath(std::string path)
{
    Creeper::SearchForLists(path);

    for (auto const& entry: std::filesystem::recursive_directory_iterator(
        path, std::filesystem::directory_options::follow_directory_symlink
            | std::filesystem::directory_options::skip_permission_denied))
    {
        if (!entry.is_regular_file())
            continue;

        std::string filePath = entry.path().string().substr(path.length());

        if (!Creeper::CheckBlackWhiteLists(filePath))
            continue;

        //stat the file
        struct stat buf;
        auto str = entry.path().string();
        auto cstr = str.c_str();
        stat(cstr, &buf);


        // make node
        FileNode node(filePath, buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size);
        fileNodes.push_back(node);
    }
}

std::vector<FileNode>* Creeper::GetResults()
{
    return &fileNodes;
}
