#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
    struct VulkanBuffer {
        vk::Buffer _M_buffer;
        vk::DeviceMemory _M_memory;
        vk::DeviceSize _M_size;
        vk::DeviceSize _M_offset = 0;
        byte* _M_mapped_data     = nullptr;


        VulkanBuffer& create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type);
        VulkanBuffer& update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);
        void* map_memory();
        VulkanBuffer& unmap_memory();
        bool is_mapped() const;
        ~VulkanBuffer();
    };

    struct VulkanVertexBuffer : RHI_VertexBuffer {
        VulkanBuffer _M_buffer;

        VulkanVertexBuffer& create(const byte* data, size_t size);

        void bind(byte stream_index, size_t offset) override;
        void update(size_t offset, size_t size, const byte* data) override;
    };

    struct VulkanIndexBuffer : public RHI_IndexBuffer {
        VulkanBuffer _M_buffer;
        vk::IndexType _M_index_type;

        VulkanIndexBuffer& create(const byte* data, size_t size, IndexBufferComponent component);

        void bind(size_t offset) override;
        void update(size_t offset, size_t size, const byte* data) override;
    };

    struct VulkanSSBO : public RHI_SSBO {
        VulkanBuffer _M_buffer;

        VulkanSSBO& create(const byte* data, size_t size);
        void bind(BindLocation location) override;
        void update(size_t offset, size_t size, const byte* data) override;
    };
}// namespace Engine
