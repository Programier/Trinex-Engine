#pragma once

namespace Trinex
{
	class RHIContext;
	class RHIBuffer;

	struct ENGINE_EXPORT RHIReadBackAllocation {
		RHIBuffer* buffer = nullptr;
		u8* data          = nullptr;
		usize offset      = 0;
		usize size        = 0;

		inline explicit operator bool() const { return buffer != nullptr && data != nullptr && size > 0; }
	};

	struct ENGINE_EXPORT RHIReadBackAllocatorStats {
		usize total_pages           = 0;
		usize free_pages            = 0;
		usize used_pages            = 0;
		usize total_capacity        = 0;
		usize free_capacity         = 0;
		usize used_capacity         = 0;
		usize current_page_used     = 0;
		usize current_page_free     = 0;
		usize current_page_capacity = 0;
	};

	class ENGINE_EXPORT RHIReadBackAllocator final
	{
	public:
		static RHIReadBackAllocation allocate(RHIContext* context, usize size, usize alignment = 16);
		static RHIReadBackAllocation allocate_chunk(RHIContext* context, usize max_size, usize alignment = 16);
		static RHIReadBackAllocatorStats statistics();
	};
}// namespace Trinex
