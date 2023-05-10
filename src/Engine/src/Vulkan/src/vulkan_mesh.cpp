#include <vulkan_api.hpp>
#include <vulkan_async_command_buffer.hpp>
#include <vulkan_mesh.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
    VulkanVertexBuffer& VulkanVertexBuffer::create(const byte* data, size_t size)
    {
        _M_instance_address = this;
        VulkanBufferBase::create(size, data, vk::BufferUsageFlagBits::eVertexBuffer);
        return *this;
    }

    VulkanVertexBuffer& VulkanVertexBuffer::bind(size_t offset)
    {
        API->current_shader()->update_descriptor_layout();
        API->_M_current_command_buffer->get()->bindVertexBuffers(0, _M_buffer, offset);
        return *this;
    }

    VulkanIndexBuffer& VulkanIndexBuffer::create(const byte* data, size_t size, IndexBufferComponent component)
    {
        _M_instance_address = this;
        _M_index_type       = get_type(component);
        VulkanBufferBase::create(size, data, vk::BufferUsageFlagBits::eIndexBuffer);
        return *this;
    }

    VulkanIndexBuffer& VulkanIndexBuffer::bind(size_t offset)
    {
        API->current_shader()->update_descriptor_layout();
        API->_M_current_command_buffer->get()->bindIndexBuffer(_M_buffer, offset, _M_index_type);
        return *this;
    }
}// namespace Engine
