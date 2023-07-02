#include <Core/engine_config.hpp>
#include <Core/engine_types.hpp>
#include <Core/exception.hpp>
#include <lz4hc.h>
#include <type_traits>



namespace Engine::Compressor
{

    template<typename Type>
    typename std::enable_if<sizeof(Type) == 1, void>::type compress(const Vector<Type>& src,
                                                                    Vector<Type>& dst)
    {
        int input_size = static_cast<int>(src.size());
        int out_size   = LZ4_compressBound(input_size);

        dst.clear();
        dst.resize(out_size);

        out_size = LZ4_compress_HC(reinterpret_cast<const char*>(src.data()), reinterpret_cast<char*>(dst.data()),
                                   input_size, out_size, engine_config.lz4_compression_level);

        dst.resize(out_size);
    }

    // Destination buffer already must have allocated memory!
    template<typename Type>
    typename std::enable_if<sizeof(Type) == 1, void>::type decompress(const Vector<Type>& src,
                                                                      Vector<Type>& dst)
    {
        int input_size = static_cast<int>(src.size());
        int out_size   = static_cast<int>(dst.size());
        out_size = LZ4_decompress_safe(reinterpret_cast<const char*>(src.data()), reinterpret_cast<char*>(dst.data()),
                                       input_size, out_size);

        if (out_size < 0)
        {
            throw EngineException("LZ4: Failed to decompress data!");
        }

        dst.resize(out_size);
    }
}// namespace Engine::Compressor
