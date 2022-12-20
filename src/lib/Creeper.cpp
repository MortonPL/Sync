#include "Lib/Creeper.h"

#include <fstream>
#include <filesystem>
#include "Utils.h"

Creeper::Creeper(std::string path)
{
    this->path = path;
}

Creeper::~Creeper()
{
}

void Creeper::SearchForLists()
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

    readList(path, ".SyncWhitelist", whitelist);
    readList(path, ".SyncBlacklist", blacklist);
}

void Creeper::CreepPath()
{
    for (auto const& entry: std::filesystem::recursive_directory_iterator(
        path, std::filesystem::directory_options::follow_directory_symlink
            | std::filesystem::directory_options::skip_permission_denied))
    {
        if (!entry.is_regular_file())
            continue;

        bool isWhitelisted = false;
        bool isBlacklisted = false;
        std::string epath = entry.path().string().substr(path.length());

        for (auto const& regex: whitelist)
        {
            if (std::regex_search(epath, regex))
            {
                isWhitelisted = true;
                break;
            }
        }

        if (!isWhitelisted)
        {
            for (auto const& regex: blacklist)
            {
                if (std::regex_search(epath, regex))
                {
                    isBlacklisted = true;
                    break;
                }
            }
            if (isBlacklisted)
            {
                continue;
            }
        }

        struct stat buf;
        auto str = entry.path().string();
        auto cstr = str.c_str();
        stat(cstr, &buf);
        FileNode node(epath, buf.st_dev, buf.st_ino, buf.st_mtim.tv_sec, buf.st_size);
        fileNodes.push_back(node);
    }
}

std::vector<FileNode>& Creeper::GetResults()
{
    return fileNodes;
}
