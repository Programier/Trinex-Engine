#pragma once
#include <vulkan_object.hpp>

#include <Core/buffer_types.hpp>
#include <vulkan_buffer.hpp>

namespace Engine
{
    struct VulkanVertexBuffer : public VulkanBufferBase
    {
        VulkanVertexBuffer& create(const byte* data, size_t size);
        VulkanVertexBuffer& bind(size_t offset);
    };

    struct VulkanIndexBuffer : public VulkanBufferBase
    {
        vk::IndexType _M_index_type;

        VulkanIndexBuffer& create(const byte* data, size_t size, IndexBufferComponent component);
        VulkanIndexBuffer& bind(size_t offset);
    };
}
