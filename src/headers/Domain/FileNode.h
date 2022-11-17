#pragma once
#include <string>

class FileNode
{
public:
    FileNode();
    ~FileNode();

private:
    std::string path;
    int flags;
};
