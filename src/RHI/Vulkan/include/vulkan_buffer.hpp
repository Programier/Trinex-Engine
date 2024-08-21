#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanBuffer {
		vk::Buffer m_buffer;
		vk::DeviceMemory m_memory;
		vk::DeviceSize m_size;
		vk::DeviceSize m_offset = 0;
		byte* m_mapped_data     = nullptr;


		VulkanBuffer& create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type);
		VulkanBuffer& update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);
		byte* map_memory();
		VulkanBuffer& unmap_memory();
		bool is_mapped() const;
		~VulkanBuffer();
	};

	struct VulkanStaticVertexBuffer : RHI_DefaultDestroyable<RHI_VertexBuffer> {
		VulkanBuffer m_buffer;

		VulkanStaticVertexBuffer& create(const byte* data, size_t size);
		void bind(byte stream_index, size_t stride, size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;
	};

	struct VulkanDynamicVertexBuffer : RHI_DefaultDestroyable<RHI_VertexBuffer> {
		Vector<VulkanBuffer> m_buffers;

		VulkanDynamicVertexBuffer& create(const byte* data, size_t size);
		void bind(byte stream_index, size_t stride, size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;
		VulkanBuffer& current();
	};

	struct VulkanIndexBuffer : public RHI_DefaultDestroyable<RHI_IndexBuffer> {
		VulkanBuffer m_buffer;
		vk::IndexType m_type;

		VulkanIndexBuffer& create(const byte* data, size_t size, IndexBufferFormat format);

		void bind(size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;
	};

	struct VulkanDynamicIndexBuffer : public RHI_DefaultDestroyable<RHI_IndexBuffer> {
		Vector<VulkanBuffer> m_buffers;
		vk::IndexType m_type;

		VulkanDynamicIndexBuffer& create(const byte* data, size_t size, IndexBufferFormat format);

		void bind(size_t offset) override;
		void update(size_t offset, size_t size, const byte* data) override;
		VulkanBuffer& current();
	};

	struct VulkanSSBO : public RHI_DefaultDestroyable<RHI_SSBO> {
		VulkanBuffer m_buffer;

		VulkanSSBO& create(const byte* data, size_t size);
		void bind(BindLocation location) override;
		void update(size_t offset, size_t size, const byte* data) override;
	};
}// namespace Engine
