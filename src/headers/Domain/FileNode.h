#pragma once
#include <string>

class FileNode
{
public:
    FileNode(std::string path);
    ~FileNode();

    std::string GetPath();

private:
    std::string path;
    int lastModified;
};
