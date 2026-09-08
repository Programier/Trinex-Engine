#pragma once
#include <Platform/enums.hpp>
#include <Platform/types.hpp>

namespace Trinex::Platform
{
	struct MemoryInfo {
		u64 total_physical     = 0;
		u64 available_physical = 0;
		u64 total_virtual      = 0;
		u64 available_virtual  = 0;
		u64 process_physical   = 0;
		u64 process_virtual    = 0;
	};

	class ENGINE_EXPORT Memory
	{
	public:
		static Memory* instance();

		virtual ~Memory() = default;

		virtual void* reserve(usize size, void* address = nullptr, VirtualMemoryFlags flags = VirtualMemoryFlags::Undefined) = 0;
		virtual bool commit(void* address, usize size, MemoryProtection protection = MemoryProtection::ReadWrite)            = 0;
		virtual bool decommit(void* address, usize size)                                                                     = 0;
		virtual bool release(void* address, usize size)                                                                      = 0;
		virtual bool protect(void* address, usize size, MemoryProtection protection)                                         = 0;
		virtual bool flush(void* address, usize size)                                                                        = 0;
		virtual bool lock(void* address, usize size)                                                                         = 0;
		virtual bool unlock(void* address, usize size)                                                                       = 0;
		virtual usize page_size() const                                                                                      = 0;
		virtual usize allocation_granularity() const                                                                         = 0;
		virtual MemoryInfo info() const                                                                                      = 0;
	};
}// namespace Trinex::Platform
