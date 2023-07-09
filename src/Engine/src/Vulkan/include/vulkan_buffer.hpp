#pragma once
#include <Core/mapped_memory.hpp>
#include <vulkan_object.hpp>

namespace Engine
{
    struct VulkanBufferBase : public VulkanObject {
    public:
        vk::Buffer _M_buffer;
        vk::DeviceMemory _M_memory;
        vk::DeviceSize _M_size;
        vk::DeviceSize _M_offset   = 0;
        byte* _M_mapped_data       = nullptr;


        VulkanBufferBase& create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type);
        VulkanBufferBase& update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size);
        MappedMemory map_memory();
        VulkanBufferBase& unmap_memory();
        bool is_mapped() const;
        ~VulkanBufferBase();
    };
}// namespace Engine
