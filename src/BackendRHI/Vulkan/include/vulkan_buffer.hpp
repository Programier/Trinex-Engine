#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanContext;

	class VulkanBuffer : public VulkanDeferredDestroy<RHIBuffer>
	{
	private:
		RHIShaderResourceView* m_srv  = nullptr;
		RHIUnorderedAccessView* m_uav = nullptr;

		vk::Buffer m_buffer        = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		size_t m_size              = 0;
		RHIDeviceAddress m_address = 0;

		RHIAccess m_access           = RHIAccess::Undefined;
		RHIBufferCreateFlags m_flags = {};

	public:
		VulkanBuffer& create(vk::DeviceSize size, RHIBufferCreateFlags flags,
		                     VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO);

		VulkanBuffer& copy(VulkanContext* ctx, vk::DeviceSize offset, const byte* data, vk::DeviceSize size);

		RHIDeviceAddress address() override;
		byte* map(RHIMappingAccess access = RHIMappingAccess::Undefined) override;
		void unmap() override;
		size_t size() const override;
		VulkanBuffer& update(VulkanContext* ctx, size_t offset, size_t size, const byte* data);
		VulkanBuffer& barrier(VulkanContext* ctx, RHIAccess access);

		RHIShaderResourceView* as_srv() override;
		RHIUnorderedAccessView* as_uav() override;
		inline RHIBufferCreateFlags flags() const { return m_flags; }
		inline vk::Buffer buffer() const { return m_buffer; }
		~VulkanBuffer();
	};

	class VulkanUniformBuffer : public VulkanBuffer
	{
		byte* m_memory;
		byte* m_block_start;
		byte* m_block_end;

	public:
		VulkanUniformBuffer* next = nullptr;

		VulkanUniformBuffer(size_t min_size = 0);
		~VulkanUniformBuffer();
		VulkanUniformBuffer& flush();
		VulkanUniformBuffer& update(const void* data, size_t size, size_t offset);


		inline VulkanUniformBuffer& reset()
		{
			m_block_start = m_block_end = m_memory;
			return *this;
		}

		inline bool contains(size_t size) const { return m_block_start + size <= m_memory + VulkanBuffer::size(); }
		inline size_t block_size() const { return m_block_end - m_block_start; }
		inline size_t block_offset() const { return m_block_start - m_memory; }
		inline byte* mapped_memory() const { return m_memory; }
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
