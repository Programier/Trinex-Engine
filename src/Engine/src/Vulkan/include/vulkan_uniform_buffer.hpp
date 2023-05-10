#pragma once
#include <vulkan_buffer.hpp>
#include <vulkan_block_allocator.hpp>

namespace Engine
{
    struct VulkanUniformBufferBlock : public VulkanBufferBase
    {
        VulkanUniformBufferBlock& create(const byte* data, size_t size);
    };


    struct VulkanUniformBuffer : public VulkanObject
    {
        BlockAllocator<VulkanUniformBufferBlock*, UNIFORM_BLOCK_SIZE>::Block _M_block;

        VulkanUniformBuffer();
        VulkanUniformBuffer& create(const byte* data, size_t size);
        VulkanUniformBuffer& update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);
        ~VulkanUniformBuffer();
    };
}
