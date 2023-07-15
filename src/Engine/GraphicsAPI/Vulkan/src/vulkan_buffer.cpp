#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>


namespace Engine
{
    VulkanBufferBase& VulkanBufferBase::create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type)
    {
        _M_size = size;
        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferDst | type, vk::MemoryPropertyFlagBits::eDeviceLocal,
                           _M_buffer, _M_memory);
        update(0, data, size);

        return *this;
    }

    VulkanBufferBase& VulkanBufferBase::update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size)
    {
        if (data == nullptr || offset >= _M_size)
            return *this;

        if (offset > _M_size)
        {
            return *this;
        }

        size                = std::min(size, _M_size - offset);
        MappedMemory memory = map_memory();
        std::memcpy(memory.data(), data, size);

        //        API->_M_current_command_buffer->get_threaded_command_buffer()->_M_buffer.updateBuffer(_M_buffer, offset, size,
        //                                                                                              data);
        return *this;
    }

    MappedMemory VulkanBufferBase::map_memory()
    {
        if (!_M_mapped_data)
        {
            _M_mapped_data = reinterpret_cast<byte*>(API->_M_device.mapMemory(_M_memory, 0, VK_WHOLE_SIZE));
        }

        return MappedMemory(_M_mapped_data, _M_size);
    }

    VulkanBufferBase& VulkanBufferBase::unmap_memory()
    {
        if (_M_mapped_data)
        {
            API->_M_device.unmapMemory(_M_memory);
            _M_mapped_data = nullptr;
        }

        return *this;
    }

    bool VulkanBufferBase::is_mapped() const
    {
        return _M_mapped_data != nullptr;
    }

    VulkanBufferBase::~VulkanBufferBase()
    {
        DESTROY_CALL(destroyBuffer, _M_buffer);
        DESTROY_CALL(freeMemory, _M_memory);
    }
}// namespace Engine
