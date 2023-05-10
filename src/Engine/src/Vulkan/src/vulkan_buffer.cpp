#include <vulkan_api.hpp>
#include <vulkan_async_command_buffer.hpp>
#include <vulkan_buffer.hpp>


namespace Engine
{
    VulkanBufferBase& VulkanBufferBase::create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type)
    {
        _M_size = size;
        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;

        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                           staging_buffer, staging_buffer_memory);

        void* vulkan_data = nullptr;
        vulkan_data       = API->_M_device.mapMemory(staging_buffer_memory, 0, size, {});

        if (data)
            std::memcpy(vulkan_data, data, size);
        else
            std::memset(vulkan_data, 0, size);
        API->_M_device.unmapMemory(staging_buffer_memory);


        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferDst | type, vk::MemoryPropertyFlagBits::eDeviceLocal,
                           _M_buffer, _M_memory);

        API->copy_buffer(staging_buffer, _M_buffer, size);

        API->_M_device.destroyBuffer(staging_buffer);
        API->_M_device.freeMemory(staging_buffer_memory);

        return *this;
    }

    VulkanBufferBase& VulkanBufferBase::update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size)
    {
        if (offset >= _M_size)
            return *this;

        API->_M_current_command_buffer->get_threaded_command_buffer()->_M_buffer.updateBuffer(_M_buffer, offset, size,
                                                                                              data);
        return *this;
    }

    MappedMemory VulkanBufferBase::map_memory()
    {
        return MappedMemory(reinterpret_cast<byte*>(API->_M_device.mapMemory(_M_memory, 0, VK_WHOLE_SIZE)), _M_size);
    }

    VulkanBufferBase& VulkanBufferBase::unmap_memory()
    {
        API->_M_device.unmapMemory(_M_memory);
        return *this;
    }

    VulkanBufferBase::~VulkanBufferBase()
    {
        DESTROY_CALL(destroyBuffer, _M_buffer);
        DESTROY_CALL(freeMemory, _M_memory);
    }
}// namespace Engine
