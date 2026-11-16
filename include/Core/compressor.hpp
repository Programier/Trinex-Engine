#include <Core/engine_types.hpp>
#include <Core/etl/array.hpp>

namespace Trinex::Compressor
{
	trinex_api void compress(const Buffer& src, Buffer& dst);
	trinex_api void decompress(const Buffer& src, Buffer& dst);
} // namespace Trinex::Compressor
