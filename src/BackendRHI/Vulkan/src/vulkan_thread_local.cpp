#include <Core/etl/algorithm.hpp>
#include <Core/etl/critical_section.hpp>
#include <vulkan_api.hpp>
#include <vulkan_thread_local.hpp>

namespace Engine
{
	VulkanThreadLocal::VulkanThreadLocal()
	{
		static CriticalSection s_section;
		ScopeLock lock(s_section);
		API->m_thread_locals.push_back(this);
	}

	VulkanThreadLocal::~VulkanThreadLocal() {}
}// namespace Engine
