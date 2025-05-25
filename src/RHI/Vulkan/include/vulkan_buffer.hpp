#pragma once
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanBuffer : public RHI_DefaultDestroyable<RHI_Buffer> {
		RHI_ShaderResourceView* m_srv  = nullptr;
		RHI_UnorderedAccessView* m_uav = nullptr;
		byte* m_mapped                 = nullptr;

		BufferCreateFlags m_flags  = {};
		vk::Buffer m_buffer        = VK_NULL_HANDLE;
		RHIAccess m_access         = RHIAccess::Undefined;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		size_t m_size              = 0;

		VulkanBuffer& create(vk::DeviceSize size, const byte* data, BufferCreateFlags flags,
		                     VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO);

		VulkanBuffer& copy(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);

		byte* map() override;
		void unmap() override;
		VulkanBuffer& update(size_t offset, size_t size, const byte* data);
		VulkanBuffer& transition(RHIAccess access);

		inline RHI_ShaderResourceView* as_srv() override { return m_srv; }
		inline RHI_UnorderedAccessView* as_uav() override { return m_uav; }
		~VulkanBuffer();
	};

	struct VulkanStaggingBuffer : VulkanBuffer {
	private:
		struct VulkanStaggingBufferManager* m_manager = nullptr;

		VulkanStaggingBuffer(VulkanStaggingBufferManager* manager);
		void destroy() override;

		friend struct VulkanStaggingBufferManager;
	};

	struct VulkanStaggingBufferManager {
	private:
		struct FreeEntry {
			struct VulkanStaggingBuffer* m_buffer = nullptr;
			size_t m_frame_number                 = 0;

			FreeEntry(VulkanStaggingBuffer* buffer, size_t frames) : m_buffer(buffer), m_frame_number(frames) {}
		};

		Set<VulkanStaggingBuffer*> m_buffers;
		Vector<FreeEntry> m_free;

	public:
		VulkanStaggingBuffer* allocate(vk::DeviceSize size, BufferCreateFlags flags);
		VulkanStaggingBufferManager& release(VulkanStaggingBuffer* buffer);
		VulkanStaggingBufferManager& update();
		~VulkanStaggingBufferManager();
	};
}// namespace Engine
