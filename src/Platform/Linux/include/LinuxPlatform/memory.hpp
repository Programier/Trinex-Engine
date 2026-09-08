#include <Platform/memory.hpp>

namespace Trinex::Platform
{
	class ENGINE_EXPORT LinuxMemory final : public Memory
	{
	private:
		LinuxMemory();

	public:
		static LinuxMemory* instance();

		void* reserve(usize size, void* address = nullptr, VirtualMemoryFlags flags = VirtualMemoryFlags::Undefined) override;
		bool commit(void* address, usize size, MemoryProtection protection = MemoryProtection::ReadWrite) override;
		bool decommit(void* address, usize size) override;
		bool release(void* address, usize size = 0) override;
		bool protect(void* address, usize size, MemoryProtection protection) override;
		bool flush(void* address, usize size) override;
		bool lock(void* address, usize size) override;
		bool unlock(void* address, usize size) override;
		usize page_size() const override;
		usize allocation_granularity() const override;
		MemoryInfo info() const override;
	};
}// namespace Trinex::Platform
