#pragma once
#include <vulkan_object.hpp>
#include <Core/mapped_memory.hpp>

namespace Engine
{
    struct VulkanBufferBase : public VulkanObject
    {
    public:
        vk::Buffer _M_buffer;
        vk::DeviceMemory _M_memory;
        vk::DeviceSize _M_size;


        VulkanBufferBase& create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type);
        VulkanBufferBase& update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);
        MappedMemory map_memory();
        VulkanBufferBase& unmap_memory();
        ~VulkanBufferBase();
    };
}
