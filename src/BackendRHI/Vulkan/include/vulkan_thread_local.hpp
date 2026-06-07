#pragma once
#include <Core/tickable.hpp>

namespace Trinex
{
	class VulkanThreadLocal : public ThreadLocalTickable
	{
	public:
		VulkanThreadLocal();
	};
}// namespace Trinex
