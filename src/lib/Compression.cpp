#include "Lib/Compression.h"

#include "Utils.h"
#include "Zstd.h"

bool Compression::Compress(std::string pathIn, std::string pathOut, off_t* compressedSize)
{
    FILE* fin;
    FILE* fout;
    void* buffIn;
    void* buffOut;
    if ((fin = fopen(pathIn.c_str(), "rb")) == NULL)
    {
        LOG(ERROR) << "Failed to open file " << pathIn << " for compression.";
        return false;
    }
    if ((fout = fopen(pathOut.c_str(), "wb")) == NULL)
    {
        LOG(ERROR) << "Failed to create file " << pathOut << " for compression.";
        fclose(fin);
        return false;
    }
    auto inSize = ZSTD_CStreamInSize();
    auto outSize = ZSTD_CStreamOutSize();
    if ((buffIn = malloc(inSize)) == nullptr)
    {
        LOG(ERROR) << "Failed to allocate buffer during compression of file " << pathIn;
        fclose(fin);
        fclose(fout);
        return false;
    }
    if ((buffOut = malloc(outSize)) == nullptr)
    {
        LOG(ERROR) << "Failed to allocate buffer during compression of file " << pathIn;
        fclose(fin);
        fclose(fout);
        free(buffIn);
        return false;
    }

    ZSTD_CCtx* context;
    if ((context = ZSTD_createCCtx()) == nullptr)
    {
        LOG(ERROR) << "Failed to create context during compression of file " << pathIn;
        fclose(fin);
        fclose(fout);
        free(buffIn);
        free(buffOut);
        return false;
    }

    ZSTD_CCtx_setParameter(context, ZSTD_c_compressionLevel, ZSTD_defaultCLevel());
    ZSTD_CCtx_setParameter(context, ZSTD_c_checksumFlag, 1);

    // read from fin to buffer, compress it, write to fout
    for(bool isLastChunk = false; !isLastChunk;)
    {
        size_t read;
        if ((read = fread(buffIn, 1, inSize, fin)) < 0)
        {
            LOG(ERROR) << "Failed to read during compression of file " << pathIn;
            ZSTD_freeCCtx(context);
            fclose(fin);
            fclose(fout);
            free(buffIn);
            free(buffOut);
            return false; 
        }
        isLastChunk = read < inSize;
        ZSTD_EndDirective mode = isLastChunk? ZSTD_e_end : ZSTD_e_continue;
        ZSTD_inBuffer input = {buffIn, read, 0};
        bool finished = false;
        do
        {
            ZSTD_outBuffer output = {buffOut, outSize, 0};
            size_t remaining = ZSTD_compressStream2(context, &output, &input, mode);
            if ((fwrite(buffOut, 1, output.pos, fout)) < 0)
            {
                LOG(ERROR) << "Failed to write during compression of file " << pathIn;
                ZSTD_freeCCtx(context);
                fclose(fout);
                fclose(fin);
                free(buffIn);
                free(buffOut);
                return false;
            }
            finished = isLastChunk? (remaining == 0) : (input.pos == input.size);
            *compressedSize += output.pos;
        } while (!finished);
    }

    ZSTD_freeCCtx(context);
    fclose(fin);
    fclose(fout);
    free(buffIn);
    free(buffOut);
    return true;
}

bool Compression::Decompress(std::string pathIn, std::string pathOut)
{
    FILE* fin;
    FILE* fout;
    void* buffIn;
    void* buffOut;
    if ((fin = fopen(pathIn.c_str(), "rb")) == NULL)
    {
        LOG(ERROR) << "Failed to open file " << pathIn << " for decompression.";
        return false;
    }
    if ((fout = fopen(pathOut.c_str(), "wb")) == NULL)
    {
        LOG(ERROR) << "Failed to create file " << pathOut << " for decompression.";
        fclose(fin);
        return false;
    }
    auto inSize = ZSTD_DStreamInSize();
    auto outSize = ZSTD_DStreamOutSize();
    if ((buffIn = malloc(inSize)) == nullptr)
    {
        LOG(ERROR) << "Failed to allocate buffer during decompression of file " << pathIn;
        fclose(fin);
        fclose(fout);
        return false;
    }
    if ((buffOut = malloc(outSize)) == nullptr)
    {
        LOG(ERROR) << "Failed to allocate buffer during decompression of file " << pathIn;
        fclose(fin);
        fclose(fout);
        free(buffIn);
        return false;
    }

    ZSTD_DCtx* context;
    if ((context = ZSTD_createDCtx()) == nullptr)
    {
        LOG(ERROR) << "Failed to create context during decompression of file " << pathIn;
        fclose(fin);
        fclose(fout);
        free(buffIn);
        free(buffOut);
        return false;
    }

    // read from fin to buffer, decompress it, write to fout
    size_t read;
    size_t ret = 0;
    bool empty = true;
    while ((read = fread(buffIn, 1, inSize, fin)))
    {
        empty = false;
        ZSTD_inBuffer input = {buffIn, read, 0};
        while (input.pos < input.size) {
            ZSTD_outBuffer output = { buffOut, outSize, 0 };
            ret = ZSTD_decompressStream(context, &output , &input);
            if (ZSTD_isError(ret))
            {
                LOG(ERROR) << "Failed to decompress file " << pathIn;
                ZSTD_freeDCtx(context);
                fclose(fout);
                fclose(fin);
                free(buffIn);
                free(buffOut);
                return false;
            }
            if ((fwrite(buffOut, 1, output.pos, fout)) < 0)
            {
                LOG(ERROR) << "Failed to write during decompression of file " << pathIn;
                ZSTD_freeDCtx(context);
                fclose(fout);
                fclose(fin);
                free(buffIn);
                free(buffOut);
                return false;
            }
        }
    }

    if (ret != 0)
    {
        LOG(ERROR) << "Unexpected EOF during decompression of file " << pathIn;
        ZSTD_freeDCtx(context);
        fclose(fout);
        fclose(fin);
        free(buffIn);
        free(buffOut);
        return false;
    }

    ZSTD_freeDCtx(context);
    fclose(fin);
    fclose(fout);
    free(buffIn);
    free(buffOut);
    return !empty;
}
