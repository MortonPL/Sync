#include "Lib/Compressor.h"

#include <fstream>

#include "Utils.h"
#include "Zstd.h"

const off_t Compressor::minimumCompressibleSize = 50000000;

struct freeContext
{
    void operator()(ZSTD_CCtx* ptr){ZSTD_freeCCtx(ptr);}
    void operator()(ZSTD_DCtx* ptr){ZSTD_freeDCtx(ptr);}
};

bool Compressor::Compress(const std::string pathIn, const std::string pathOut, off_t& compressedSize)
{
    try
    {
        std::ifstream inputStream(pathIn, std::ios::binary);
        std::ofstream outputStream(pathOut, std::ios::binary);

        const auto inSize = ZSTD_CStreamInSize();
        const auto outSize = ZSTD_CStreamOutSize();
        std::vector<char> buffIn(inSize, 0);
        std::vector<char> buffOut(outSize, 0);

        auto context = std::unique_ptr<ZSTD_CCtx, freeContext>(ZSTD_createCCtx(), freeContext());
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_compressionLevel, ZSTD_defaultCLevel());
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_checksumFlag, 1);

        // NOTE: reading less than inSize bytes sets failbit, so we can't put read() as the condiiton!
        inputStream.read(buffIn.data(), inSize);
        size_t len;
        while((len = !inputStream.eof()? inSize: inputStream.gcount()) > 0)
        {
            bool isLastChunk = len < inSize;
            ZSTD_EndDirective mode = isLastChunk? ZSTD_e_end : ZSTD_e_continue;
            ZSTD_inBuffer input = {buffIn.data(), len, 0};
            bool finished = false;
            while (!finished)
            {
                ZSTD_outBuffer output = {buffOut.data(), outSize, 0};
                size_t result = ZSTD_compressStream2(context.get(), &output, &input, mode);
                if (ZSTD_isError(result))
                    throw std::runtime_error("Compression error.");
                outputStream.write(buffOut.data(), output.pos);
                finished = isLastChunk? (result == 0) : (input.pos == input.size);
                compressedSize += output.pos;
            }
            inputStream.read(buffIn.data(), inSize);
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to compress file " << pathIn << ". Reason: " << e.what();
        return false;
    }

    return true;
}

bool Compressor::Decompress(const std::string pathIn, const std::string pathOut)
{
    bool empty = true;

    try
    {
        std::ifstream inputStream(pathIn, std::ios::binary);
        std::ofstream outputStream(pathOut, std::ios::binary);

        const auto inSize = ZSTD_DStreamInSize();
        const auto outSize = ZSTD_DStreamOutSize();
        std::vector<char> buffIn(inSize, 0);
        std::vector<char> buffOut(outSize, 0);

        auto context = std::unique_ptr<ZSTD_DCtx, freeContext>(ZSTD_createDCtx(), freeContext());

        // same NOTE as in Compress()
        size_t len;
        inputStream.read(buffIn.data(), inSize);
        while ((len = !inputStream.eof()? inSize: inputStream.gcount()) > 0)
        {
            empty = false;
            ZSTD_inBuffer input = {buffIn.data(), len, 0};
            while (input.pos < input.size)
            {
                ZSTD_outBuffer output = {buffOut.data(), outSize, 0 };
                size_t result = ZSTD_decompressStream(context.get(), &output , &input);
                if (ZSTD_isError(result))
                    throw std::runtime_error("Compression error.");
                outputStream.write(buffOut.data(), output.pos);
            }
            inputStream.read(buffIn.data(), inSize);
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to decompress file " << pathIn << ". Reason: " << e.what();
        return false;
    }

    return !empty;
}
