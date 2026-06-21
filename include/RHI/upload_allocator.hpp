#pragma once
#include <Core/engine_types.hpp>
#include <RHI/enums.hpp>
#include <cstring>

namespace Trinex
{
	class RHIBuffer;
	class RHIContext;

	struct ENGINE_EXPORT RHIUploadAllocation {
		RHIBuffer* buffer = nullptr;
		u8* data          = nullptr;
		usize offset      = 0;
		usize size        = 0;

		inline explicit operator bool() const { return buffer != nullptr && data != nullptr && size > 0; }

		inline RHIUploadAllocation& copy(const void* src, usize bytes, usize dst_offset = 0)
		{
			std::memcpy(data + dst_offset, src, bytes);
			return *this;
		}
	};

	struct ENGINE_EXPORT RHIUploadAllocatorStats {
		usize total_pages         = 0;
		usize free_pages          = 0;
		usize used_pages          = 0;
		usize total_capacity      = 0;
		usize free_capacity       = 0;
		usize used_capacity       = 0;
		usize current_page_used   = 0;
		usize current_page_free   = 0;
		usize current_page_capacity = 0;
	};

	class ENGINE_EXPORT RHIUploadAllocator final
	{
	public:
		static RHIUploadAllocation allocate(RHIContext* context, usize size, usize alignment = 16);
		static RHIUploadAllocatorStats statistics();
	};
}// namespace Trinex
