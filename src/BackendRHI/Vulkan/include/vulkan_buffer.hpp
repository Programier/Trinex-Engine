#pragma once
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanBuffer : public VulkanDeferredDestroy<RHI_Buffer>
	{
	private:
		RHI_ShaderResourceView* m_srv  = nullptr;
		RHI_UnorderedAccessView* m_uav = nullptr;

		RHIBufferCreateFlags m_flags = {};
		vk::Buffer m_buffer          = VK_NULL_HANDLE;
		RHIAccess m_access           = RHIAccess::Undefined;
		VmaAllocation m_allocation   = VK_NULL_HANDLE;
		size_t m_size                = 0;

	public:
		VulkanBuffer& create(vk::DeviceSize size, const byte* data, RHIBufferCreateFlags flags,
		                     VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO);

		VulkanBuffer& copy(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);

		byte* map() override;
		void unmap() override;
		VulkanBuffer& update(size_t offset, size_t size, const byte* data);
		VulkanBuffer& transition(RHIAccess access);

		inline RHI_ShaderResourceView* as_srv() override { return m_srv; }
		inline RHI_UnorderedAccessView* as_uav() override { return m_uav; }
		inline size_t size() const { return m_size; }
		inline RHIBufferCreateFlags flags() const { return m_flags; }
		inline vk::Buffer buffer() const { return m_buffer; }
		~VulkanBuffer();
	};

	class VulkanStaggingBuffer : public VulkanBuffer
	{
	private:
		class VulkanStaggingBufferManager* m_manager = nullptr;

		VulkanStaggingBuffer(VulkanStaggingBufferManager* manager);
		void destroy() override;

		friend class VulkanStaggingBufferManager;
	};

	class VulkanStaggingBufferManager
	{
	private:
		struct FreeEntry {
			VulkanStaggingBuffer* m_buffer = nullptr;
			size_t m_frame_number          = 0;

			FreeEntry(VulkanStaggingBuffer* buffer, size_t frames) : m_buffer(buffer), m_frame_number(frames) {}
		};

		Set<VulkanStaggingBuffer*> m_buffers;
		Vector<FreeEntry> m_free;

	public:
		VulkanStaggingBuffer* allocate(vk::DeviceSize size, RHIBufferCreateFlags flags);
		VulkanStaggingBufferManager& release(VulkanStaggingBuffer* buffer);
		VulkanStaggingBufferManager& update();
		~VulkanStaggingBufferManager();
	};
}// namespace Engine
