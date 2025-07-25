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
	class VulkanBuffer : public VulkanDeferredDestroy<RHIBuffer>
	{
	private:
		FlatMap<uint64_t, RHIShaderResourceView*> m_srv;
		FlatMap<uint64_t, RHIUnorderedAccessView*> m_uav;

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

		RHIShaderResourceView* as_srv(uint32_t offset = 0, uint32_t size = 0) override;
		RHIUnorderedAccessView* as_uav(uint32_t offset = 0, uint32_t size = 0) override;
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
