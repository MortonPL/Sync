#include "Lib/Compression.h"

#include <fstream>

#include "Utils.h"
#include "Zstd.h"

const off_t Compression::minimumCompressibleSize = 50000000;

struct freeContext
{
    void operator()(ZSTD_CCtx* ptr){ZSTD_freeCCtx(ptr);}
    void operator()(ZSTD_DCtx* ptr){ZSTD_freeDCtx(ptr);}
};

bool Compression::Compress(const std::string pathIn, const std::string pathOut, off_t& compressedSize)
{
    try
    {
        std::ifstream inputStream(pathIn, std::ios::binary);
        std::ofstream outputStream(pathOut, std::ios::binary);
        inputStream.exceptions(std::ifstream::failbit|std::ifstream::badbit);
        outputStream.exceptions(std::ofstream::failbit|std::ofstream::badbit);

        const auto inSize = ZSTD_CStreamInSize();
        const auto outSize = ZSTD_CStreamOutSize();
        std::vector<char> buffIn(inSize, 0);
        std::vector<char> buffOut(outSize, 0);

        auto context = std::unique_ptr<ZSTD_CCtx, freeContext>(ZSTD_createCCtx(), freeContext());
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_compressionLevel, ZSTD_defaultCLevel());
        ZSTD_CCtx_setParameter(context.get(), ZSTD_c_checksumFlag, 1);

        for(bool isLastChunk = false; !isLastChunk;)
        {
            inputStream.read(buffIn.data(), inSize);
            auto read = inputStream? inSize: inputStream.gcount();
            isLastChunk = read < inSize;
            ZSTD_EndDirective mode = isLastChunk? ZSTD_e_end : ZSTD_e_continue;
            ZSTD_inBuffer input = {buffIn.data(), read, 0};
            bool finished = false;
            do
            {
                ZSTD_outBuffer output = {buffOut.data(), outSize, 0};
                size_t result = ZSTD_compressStream2(context.get(), &output, &input, mode);
                if (ZSTD_isError(result))
                    throw std::runtime_error("Compression error.");
                outputStream.write(buffOut.data(), output.pos);
                finished = isLastChunk? (result == 0) : (input.pos == input.size);
                compressedSize += output.pos;
            } while (!finished);
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "Failed to compress file " << pathIn << ". Reason: " << e.what();
        return false;
    }

    return true;
}

bool Compression::Decompress(const std::string pathIn, const std::string pathOut)
{
    bool empty = true;

    try
    {
        std::ifstream inputStream(pathIn, std::ios::binary);
        std::ofstream outputStream(pathOut, std::ios::binary);
        inputStream.exceptions(std::ifstream::failbit|std::ifstream::badbit);
        outputStream.exceptions(std::ofstream::failbit|std::ofstream::badbit);

        const auto inSize = ZSTD_DStreamInSize();
        const auto outSize = ZSTD_DStreamOutSize();
        std::vector<char> buffIn(inSize, 0);
        std::vector<char> buffOut(outSize, 0);

        auto context = std::unique_ptr<ZSTD_DCtx, freeContext>(ZSTD_createDCtx(), freeContext());

        size_t read;
        inputStream.read(buffIn.data(), inSize);
        while ((read = inputStream? inSize: inputStream.gcount()) > 0)
        {
            empty = false;
            ZSTD_inBuffer input = {buffIn.data(), read, 0};
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
