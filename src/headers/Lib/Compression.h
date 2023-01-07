#pragma once
#include <string>

namespace Compression
{
    bool Compress(std::string pathIn, std::string pathOut, off_t* compressedSize);
    bool Decompress(std::string pathIn, std::string pathOut);
}
