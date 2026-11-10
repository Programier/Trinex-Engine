#pragma once
#include <RHI/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanMemory final : public RHIMemory
	{
	private:
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		vk::DeviceMemory m_memory;
		usize m_size;
		RHIMemoryType m_type;

	public:
		VulkanMemory(usize size, RHIMemoryType type);
		~VulkanMemory();

		RHI* device() const override;
		u8* map(RHIMappingAccess access) override;
		void unmap() override;
		usize size() const override;
		RHIMemoryType type() const override;

		inline vk::DeviceMemory memory() const { return m_memory; }
		inline VmaAllocation allocation() const { return m_allocation; }
	};
} // namespace Trinex
