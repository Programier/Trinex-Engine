#pragma once
#include <Graphics/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanBuffer : public RHI_DefaultDestroyable<RHI_Object> {
		vk::BufferUsageFlags m_usage = {};
		vk::Buffer m_buffer          = VK_NULL_HANDLE;
		VmaAllocation m_allocation   = VK_NULL_HANDLE;
		size_t m_size                = 0;
		byte* m_mapped               = nullptr;

		VulkanBuffer& create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlags usage,
		                     VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO);
		VulkanBuffer& copy(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);
		VulkanBuffer& copy(vk::DeviceSize offset, VulkanBuffer* buffer, vk::DeviceSize size);
		VulkanBuffer& update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);
		byte* map_memory();
		VulkanBuffer& unmap_memory();
		~VulkanBuffer();
	};

	struct VulkanStaggingBufferManager {
	private:
		struct FreeEntry {
			VulkanBuffer* m_buffer = nullptr;
			size_t m_frame_number  = 0;

			FreeEntry(VulkanBuffer* buffer, size_t frames) : m_buffer(buffer), m_frame_number(frames)
			{}
		};

		Set<VulkanBuffer*> m_buffers;
		Vector<FreeEntry> m_free;

	public:
		VulkanBuffer* allocate(vk::DeviceSize size, vk::BufferUsageFlags usage,
		                       VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_CPU_ONLY);
		VulkanStaggingBufferManager& release(VulkanBuffer* buffer);
		VulkanStaggingBufferManager& update();
		~VulkanStaggingBufferManager();
	};

	struct VulkanVertexBuffer : RHI_DefaultDestroyable<RHI_VertexBuffer> {
		VulkanBuffer m_buffer;

		VulkanVertexBuffer& create(const byte* data, size_t size);
		void bind(byte stream_index, size_t stride, size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;
	};

	struct VulkanIndexBuffer : public RHI_DefaultDestroyable<RHI_IndexBuffer> {
		VulkanBuffer m_buffer;
		vk::IndexType m_type;

		VulkanIndexBuffer& create(const byte* data, size_t size, IndexBufferFormat format);

		void bind(size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;
	};

	struct VulkanSSBO : public RHI_DefaultDestroyable<RHI_SSBO> {
		VulkanBuffer m_buffer;

		VulkanSSBO& create(const byte* data, size_t size);
		void bind(BindLocation location) override;
		void update(size_t offset, size_t size, const byte* data) override;
	};
}// namespace Engine
