#include <Core/memory.hpp>

namespace Engine
{
	ENGINE_EXPORT void* memcpy_elements(void* dst, const void* src, size_t element_size, size_t element_count, size_t dst_stride,
	                                    size_t src_stride)
	{
		if (src_stride == 0)
			src_stride = element_size;

		if (dst_stride == 0)
			dst_stride = element_size;

		if (src_stride == dst_stride && src_stride == element_size)
			return memcpy(dst, src, element_size * element_count);

		auto* d = static_cast<uint8_t*>(dst);
		auto* s = static_cast<const uint8_t*>(src);

		for (size_t i = 0; i < element_count; ++i)
		{
			memcpy(d, s, element_size);
			d += dst_stride;
			s += src_stride;
		}

		return dst;
	}

	ENGINE_EXPORT void* memcpy_transform(void* dst, const void* src, size_t element_count, size_t dst_stride, size_t src_stride,
	                                     void (*transform)(void* dst, const void* src))
	{
		auto* d = static_cast<uint8_t*>(dst);
		auto* s = static_cast<const uint8_t*>(src);

		for (size_t i = 0; i < element_count; ++i)
		{
			transform(d, s);
			d += dst_stride;
			s += src_stride;
		}

		return dst;
	}

	ENGINE_EXPORT const byte* memory_search(const byte* haystack, size_t haystack_len, const byte* needle, size_t needle_len)
	{
		if (needle_len > haystack_len)
		{
			return nullptr;
		}

		if (needle_len == 0)
			return haystack;

		for (size_t i = 0, count = haystack_len - needle_len; i <= count; ++i)
		{
			if (memcmp(haystack + i, needle, needle_len) == 0)
			{
				return haystack + i;
			}
		}

		return nullptr;
	}
}// namespace Engine
