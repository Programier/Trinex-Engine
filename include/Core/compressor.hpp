#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>

namespace Engine::Compressor
{
	ENGINE_EXPORT void compress(const Buffer& src, Buffer& dst);
	ENGINE_EXPORT void decompress(const Buffer& src, Buffer& dst);
}// namespace Engine::Compressor
