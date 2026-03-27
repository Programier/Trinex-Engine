#include <Core/math/math.hpp>
#include <Core/memory.hpp>

namespace Trinex
{
	ENGINE_EXPORT void* memcpy_elements(void* dst, const void* src, usize element_size, usize element_count, usize dst_stride,
	                                    usize src_stride)
	{
		if (src_stride == 0)
			src_stride = element_size;

		if (dst_stride == 0)
			dst_stride = element_size;

		if (src_stride == dst_stride && src_stride == element_size)
			return memcpy(dst, src, element_size * element_count);

		auto* d = static_cast<u8*>(dst);
		auto* s = static_cast<const u8*>(src);

		element_size = Math::min(dst_stride, src_stride);

		for (usize i = 0; i < element_count; ++i)
		{
			memcpy(d, s, element_size);
			d += dst_stride;
			s += src_stride;
		}

		return dst;
	}

	ENGINE_EXPORT void* memcpy_transform(void* dst, const void* src, usize element_count, usize dst_stride, usize src_stride,
	                                     void (*transform)(void* dst, const void* src))
	{
		auto* d = static_cast<u8*>(dst);
		auto* s = static_cast<const u8*>(src);

		for (usize i = 0; i < element_count; ++i)
		{
			transform(d, s);
			d += dst_stride;
			s += src_stride;
		}

		return dst;
	}

	ENGINE_EXPORT const u8* memory_search(const u8* haystack, usize haystack_len, const u8* needle, usize needle_len)
	{
		if (needle_len > haystack_len)
		{
			return nullptr;
		}

		if (needle_len == 0)
			return haystack;

		for (usize i = 0, count = haystack_len - needle_len; i <= count; ++i)
		{
			if (memcmp(haystack + i, needle, needle_len) == 0)
			{
				return haystack + i;
			}
		}

		return nullptr;
	}
}// namespace Trinex
