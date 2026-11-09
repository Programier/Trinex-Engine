#include <vulkan_api.hpp>
#include <vulkan_memory.hpp>

namespace Trinex
{
	VulkanMemory::VulkanMemory(usize size, RHIMemoryType type) : m_size(size), m_type(type)
	{
		VkMemoryRequirements requirements = {};
		requirements.size                 = size;
		requirements.alignment            = 1;
		requirements.memoryTypeBits       = ~0u;

		VmaAllocationCreateInfo create_info = {};
		create_info.flags                   = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;

		switch (type)
		{
			case RHIMemoryType::DeviceLocal: create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; break;

			case RHIMemoryType::Upload:
				create_info.requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				create_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;

			case RHIMemoryType::Readback:
				create_info.requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
				create_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
				break;
		}

		const VkResult result =
		        vmaAllocateMemory(VulkanAPI::instance()->m_allocator, &requirements, &create_info, &m_allocation, nullptr);
		trinex_assert_msg(result == VK_SUCCESS, "Failed to allocate Vulkan memory");
	}

	VulkanMemory::~VulkanMemory()
	{
		if (m_allocation)
			vmaFreeMemory(VulkanAPI::instance()->m_allocator, m_allocation);
	}

	RHI* VulkanMemory::device() const
	{
		return VulkanAPI::instance();
	}

	usize VulkanMemory::size() const
	{
		return m_size;
	}

	RHIMemoryType VulkanMemory::type() const
	{
		return m_type;
	}

	RHIMemory* VulkanAPI::create_memory(usize size, RHIMemoryType type)
	{
		return trx_new VulkanMemory(size, type);
	}
} // namespace Trinex
