#include <Core/etl/algorithm.hpp>
#include <Core/etl/critical_section.hpp>
#include <vulkan_api.hpp>
#include <vulkan_thread_local.hpp>

namespace Trinex
{
	VulkanThreadLocal::VulkanThreadLocal()
	{
		[[maybe_unused]] auto lock = critical_section();
		VulkanAPI::instance()->m_thread_locals.push_back(this);
	}
}// namespace Trinex
