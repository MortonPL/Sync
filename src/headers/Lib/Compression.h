#pragma once
#include <string>

namespace Compression
{
    bool Compress(const std::string pathIn, const std::string pathOut, off_t& compressedSize);
    bool Decompress(const std::string pathIn, const std::string pathOut);
}
