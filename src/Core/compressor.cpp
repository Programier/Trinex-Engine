#include <Core/compressor.hpp>
#include <Engine/settings.hpp>
#include <lz4hc.h>

namespace Trinex::Compressor
{

	ENGINE_EXPORT void compress(const Buffer& src, Buffer& dst)
	{
		int input_size = static_cast<int>(src.size());
		int out_size   = LZ4_compressBound(input_size);

		dst.clear();
		dst.resize(out_size + sizeof(usize));

		out_size = LZ4_compress_HC(reinterpret_cast<const char*>(src.data()), reinterpret_cast<char*>(dst.data() + sizeof(usize)),
		                           input_size, out_size, Settings::lz4_compression_level);

		dst.resize(out_size + sizeof(usize));
		(*reinterpret_cast<usize*>(dst.data())) = src.size();
	}

	ENGINE_EXPORT void decompress(const Buffer& src, Buffer& dst)
	{
		int input_size = static_cast<int>(src.size() - sizeof(usize));
		dst.resize(*reinterpret_cast<const usize*>(src.data()));
		int out_size = static_cast<int>(dst.size());

		out_size = LZ4_decompress_safe(reinterpret_cast<const char*>(src.data() + sizeof(usize)),
		                               reinterpret_cast<char*>(dst.data()), input_size, out_size);
		trinex_verify(out_size >= 0);
		dst.resize(out_size);
	}
}// namespace Trinex::Compressor
