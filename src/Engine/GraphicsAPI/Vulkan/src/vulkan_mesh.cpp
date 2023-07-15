#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_mesh.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_types.hpp>

namespace Engine
{


    VulkanVertexBuffer& VulkanVertexBuffer::create(const byte* data, size_t size)
    {
        VulkanBufferBase::create(size, data, vk::BufferUsageFlagBits::eVertexBuffer);
        return *this;
    }

    VulkanVertexBuffer& VulkanVertexBuffer::bind(size_t offset)
    {
        if (API->_M_state->_M_vertex_buffer != this)
        {
            API->_M_command_buffer->get().bindVertexBuffers(0, _M_buffer, offset);
            API->_M_state->_M_vertex_buffer = this;
        }

        return *this;
    }

    VulkanIndexBuffer& VulkanIndexBuffer::create(const byte* data, size_t size, IndexBufferComponent component)
    {
        _M_index_type = get_type(component);
        VulkanBufferBase::create(size, data, vk::BufferUsageFlagBits::eIndexBuffer);
        return *this;
    }

    VulkanIndexBuffer& VulkanIndexBuffer::bind(size_t offset)
    {
        if (API->_M_state->_M_index_buffer != this)
        {
            API->_M_command_buffer->get().bindIndexBuffer(_M_buffer, offset, _M_index_type);
            API->_M_state->_M_index_buffer = this;
        }

        return *this;
    }
}// namespace Engine
