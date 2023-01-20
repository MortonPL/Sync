#ifndef LIB_COMPRESSOR_H
#define LIB_COMPRESSOR_H
#include <string>

namespace Compressor
{
    extern const off_t minimumCompressibleSize;

    bool Compress(const std::string pathIn, const std::string pathOut, off_t& compressedSize);
    bool Decompress(const std::string pathIn, const std::string pathOut);
}

#endif
