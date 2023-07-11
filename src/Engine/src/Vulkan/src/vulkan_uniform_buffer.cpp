#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_uniform_buffer.hpp>

namespace Engine
{
    VulkanUniformBufferBlock& VulkanUniformBufferBlock::create(const byte* data, size_t size)
    {
        VulkanBufferBase::create(size, data, vk::BufferUsageFlagBits::eUniformBuffer);
        return *this;
    }

    VulkanUniformBuffer::VulkanUniformBuffer()
    {}

    VulkanUniformBuffer& VulkanUniformBuffer::create(const byte* data, size_t size)
    {
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
        return _M_buffer[API->_M_current_buffer];
    }

    VulkanUniformBufferMap::~VulkanUniformBufferMap()
    {
        for (VulkanUniformBuffer* buffer : _M_buffer)
        {
            delete buffer;
        }
    }
}// namespace Engine
