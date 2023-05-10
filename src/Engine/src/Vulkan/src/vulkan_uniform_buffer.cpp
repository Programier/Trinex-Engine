#include <vulkan_api.hpp>
#include <vulkan_async_command_buffer.hpp>
#include <vulkan_uniform_buffer.hpp>

namespace Engine
{
    VulkanUniformBufferBlock& VulkanUniformBufferBlock::create(const byte* data, size_t size)
    {
        _M_instance_address = this;
        VulkanBufferBase::create(size, data, vk::BufferUsageFlagBits::eUniformBuffer);
        return *this;
    }

    VulkanUniformBuffer::VulkanUniformBuffer()
    {
        _M_instance_address = this;
    }

    VulkanUniformBuffer& VulkanUniformBuffer::create(const byte* data, size_t size)
    {
        _M_block = API->_M_uniform_allocator.allocate(size);

        void* vulkan_memory =
                API->_M_device.mapMemory(_M_block._M_block->_M_memory, _M_block._M_offset, _M_block._M_size);

        if (data)
            std::memcpy(vulkan_memory, data, size);
        else
            std::memset(vulkan_memory, 0, size);
        API->_M_device.unmapMemory(_M_block._M_block->_M_memory);

        return *this;
    }

    VulkanUniformBuffer& VulkanUniformBuffer::update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size)
    {
        if (!data)
            return *this;
        _M_block._M_block->update(_M_block._M_offset + offset, data, size);
        return *this;
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        if (_M_block._M_size != 0)
        {
            API->_M_uniform_allocator.deallocate(_M_block);
        }
    }

}// namespace Engine
