#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/flat_map.hpp>
#include <Core/etl/set.hpp>
#include <RHI/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanContext;
	class VulkanMemory;

	class VulkanBuffer : public RHIBuffer
	{
	private:
		RHIShaderResourceView* m_srv  = nullptr;
		RHIUnorderedAccessView* m_uav = nullptr;

		VmaAllocation m_allocation = VK_NULL_HANDLE;
		vk::Buffer m_buffer        = VK_NULL_HANDLE;
		RHIDeviceAddress m_address = 0;
		usize m_size               = 0;
		usize m_offset             = 0;

		RHIAccess m_access     = RHIAccess::Undefined;
		RHIBufferFlags m_flags = {};

	public:
		VulkanBuffer& create(vk::DeviceSize size, RHIBufferFlags flags, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO);

		VulkanBuffer& copy(VulkanContext* ctx, vk::DeviceSize offset, const u8* data, vk::DeviceSize size);

		RHI* device() const override;
		usize size() const override;
		RHIDeviceAddress address() override;
		u8* map(RHIMappingAccess access = RHIMappingAccess::Undefined) override;
		void unmap() override;
		VulkanBuffer& barrier(VulkanContext* ctx, RHIAccess access);

		RHIShaderResourceView* as_srv(RHIBufferViewType view, RHIColorFormat format = RHIColorFormat::Undefined) override;
		RHIUnorderedAccessView* as_uav(RHIBufferViewType view, RHIColorFormat format = RHIColorFormat::Undefined) override;
		inline RHIBufferFlags flags() const { return m_flags; }
		inline vk::Buffer buffer() const { return m_buffer; }
		~VulkanBuffer();
	};
} // namespace Trinex
