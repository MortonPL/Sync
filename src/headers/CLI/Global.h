#ifndef SRC_CLI_GLOBAL_H
#define SRC_CLI_GLOBAL_H
#include <string>

class GlobalCLI
{
public:
    enum CLIMode
    {
        Default = 0x000,
        Creep = 0x001,
        DaemonServant = 0x002,
        HomePath = 0x004,
        Stat = 0x008,
        Unblock = 0x010,
        Compress = 0x020,
        Decompress = 0x040,
    };

    static int mode;
    static std::string pathCreep;
    static std::string pathBlock;
    static std::string pathStat;
    static std::string pathUnblock;
    static std::string pathCompressIn;
    static std::string pathCompressOut;
    static std::string pathDecompressIn;
    static std::string pathDecompressOut;
};

#endif
