#ifndef LIB_COMPRESSION_H
#define LIB_COMPRESSION_H
#include <string>

namespace Compression
{
    extern const off_t minimumCompressibleSize;

    bool Compress(const std::string pathIn, const std::string pathOut, off_t& compressedSize);
    bool Decompress(const std::string pathIn, const std::string pathOut);
}

#endif
