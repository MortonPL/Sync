#ifndef LIB_CREEPER_H
#define LIB_CREEPER_H
#include <list>
#include <forward_list>
#include <filesystem>
#include <regex>

#include "Domain/FileNode.h"

/*Recursively creeps through a directory and gathers FileNodes. Also provides an option to
check a path against blacklist/whitelist rules.*/
class Creeper
{
public:
    enum class Result: char
    {
        Ok = 0,
        Error,
        Permissions,
        NotExists,
        NotADir,
        Block,
    };

    Creeper();
    ~Creeper();
    Creeper::Result CreepPath(std::string rootPath, std::forward_list<FileNode>& fileNodes);
    static Creeper::Result MakeSingleNode(const std::string& path, FileNode& node);
    static Creeper::Result MakeSingleNodeLight(const std::string& path, FileNode& node);
    size_t GetResultsCount() const;
    bool CheckIfFileIsIgnored(const std::string& path) const;

    static std::string SyncBlackListFile;
    static std::string SyncWhiteListFile;

private:
    class HasherState
    {
    public:
        HasherState();
        ~HasherState();

        struct hasherFree
        {
            void operator()(XXH3_state_t* ptr){XXH3_freeState(ptr);}
        };

        std::unique_ptr<XXH3_state_t, hasherFree> pState;
        std::vector<char> buffer;
        static const int bufferSize = 1024 * 32;
    };

    void SearchForLists(const std::string& path);
    bool MakeNode(const std::filesystem::directory_entry& entry,
                  const std::string& rootPath, HasherState& state, FileNode& node) const;
    static XXH128_hash_t HashFile(HasherState& state, const std::string& path);
    static Creeper::Result CheckIfPathExists(const std::string& path, struct stat& buf, const bool isDir=false);

    std::size_t fileNodesCount = 0;
    std::list<std::regex> whitelist;
    std::list<std::regex> blacklist;
};

#endif
