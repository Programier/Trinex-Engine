#include <Core/etl/limits.hpp>
#include <Platform/memory.hpp>

namespace Trinex::Platform
{
	VirtualMemoryBlock Memory::allocate(usize size)
	{
		const usize alignment = granularity();
		if (size == 0 || alignment == 0 || size > etl::numeric_limits<usize>::max() - (alignment - 1))
			return {};

		size = (size + alignment - 1) & ~(alignment - 1);

		void* memory = reserve(size);
		if (!memory)
			return {};

		if (!commit(memory, size))
		{
			release(memory, size);
			return {};
		}

		return {
		        .memory = memory,
		        .size   = size,
		};
	}

	void Memory::release(VirtualMemoryBlock block)
	{
		if (block)
		{
			release(block.memory, block.size);
		}
	}
} // namespace Trinex::Platform
