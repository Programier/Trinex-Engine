#include <Core/compressor.hpp>
#include <Core/engine_config.hpp>
#include <lz4hc.h>
#include <Core/exception.hpp>

namespace Engine::Compressor
{

    ENGINE_EXPORT void compress(const Buffer& src, Buffer& dst)
    {
        int input_size = static_cast<int>(src.size());
        int out_size   = LZ4_compressBound(input_size);

        dst.clear();
        dst.resize(out_size);

        out_size = LZ4_compress_HC(reinterpret_cast<const char*>(src.data()), reinterpret_cast<char*>(dst.data()), input_size,
                                   out_size, engine_config.lz4_compression_level);

        dst.resize(out_size);
    }

    ENGINE_EXPORT void decompress(const Buffer& src, Buffer& dst)
    {
        int input_size = static_cast<int>(src.size());
        int out_size   = static_cast<int>(dst.size());
        out_size = LZ4_decompress_safe(reinterpret_cast<const char*>(src.data()), reinterpret_cast<char*>(dst.data()), input_size,
                                       out_size);

        if (out_size < 0)
        {
            throw EngineException("LZ4: Failed to decompress data!");
        }

        dst.resize(out_size);
    }
}// namespace Engine::Compressor