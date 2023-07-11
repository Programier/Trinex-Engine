#pragma once
#include <vulkan_block_allocator.hpp>
#include <vulkan_buffer.hpp>

namespace Engine
{
    struct VulkanUniformBufferBlock : public VulkanBufferBase {
        VulkanUniformBufferBlock& create(const byte* data, size_t size);
    };


    struct VulkanUniformBuffer : public VulkanBufferBase {
        VulkanUniformBuffer();
        VulkanUniformBuffer& create(const byte* data, size_t size);
        VulkanUniformBuffer& update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);
        ~VulkanUniformBuffer();
    };

    struct VulkanUniformBufferMap : public VulkanObject {
        Vector<VulkanUniformBuffer*> _M_buffer;

        VulkanUniformBufferMap(const byte* data, size_t size);
        VulkanUniformBuffer* current_buffer();
        ~VulkanUniformBufferMap();
    };
}// namespace Engine
