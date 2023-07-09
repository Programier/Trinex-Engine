#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
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
        //        _M_block = API->_M_uniform_allocator.allocate(size);

        //        void* vulkan_memory =
        //                API->_M_device.mapMemory(_M_block._M_block->_M_memory, _M_block._M_offset, _M_block._M_size);

        //        if (data)
        //            std::memcpy(vulkan_memory, data, size);
        //        else
        //            std::memset(vulkan_memory, 0, size);
        //        API->_M_device.unmapMemory(_M_block._M_block->_M_memory);

        VulkanBufferBase::create(size, data, vk::BufferUsageFlagBits::eUniformBuffer);
        return *this;
    }

    VulkanUniformBuffer& VulkanUniformBuffer::update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size)
    {
        if (!data)
            return *this;
        VulkanBufferBase::update(offset, data, size);
        return *this;
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {}

    VulkanUniformBufferMap::VulkanUniformBufferMap(const byte* data, size_t size)
    {
        _M_buffer.reserve(MAIN_FRAMEBUFFERS_COUNT);
        for (int i = 0; i < MAIN_FRAMEBUFFERS_COUNT; i++)
        {
            VulkanUniformBuffer* buffer = new VulkanUniformBuffer();
            buffer->create(data, size);
            _M_buffer.push_back(buffer);
        }
    }

    VulkanUniformBuffer* VulkanUniformBufferMap::current_buffer()
    {
        return _M_buffer[API->_M_current_frame];
    }

    VulkanUniformBuffer* VulkanUniformBufferMap::next_buffer()
    {
        return _M_buffer[API->_M_next_frame];
    }

    VulkanUniformBufferMap::~VulkanUniformBufferMap()
    {
        for (VulkanUniformBuffer* buffer : _M_buffer)
        {
            delete buffer;
        }
    }

}// namespace Engine
