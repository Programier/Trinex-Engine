#include <Graphics/shader_parameters.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_state.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
    VulkanBuffer& VulkanBuffer::create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type)
    {
        m_size = size;
        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferDst | type, vk::MemoryPropertyFlagBits::eHostVisible, m_buffer,
                           m_memory);
        update(0, data, size);
        return *this;
    }

    VulkanBuffer& VulkanBuffer::update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size)
    {
        if (data == nullptr || offset >= m_size)
            return *this;

        if (offset > m_size)
        {
            return *this;
        }

        size         = std::min(size, m_size - offset);
        byte* memory = map_memory();
        std::memcpy(memory + offset, data, size);
        return *this;
    }

    byte* VulkanBuffer::map_memory()
    {
        if (!m_mapped_data)
        {
            m_mapped_data = reinterpret_cast<byte*>(API->m_device.mapMemory(m_memory, 0, VK_WHOLE_SIZE));
        }

        return m_mapped_data;
    }

    VulkanBuffer& VulkanBuffer::unmap_memory()
    {
        if (m_mapped_data)
        {
            API->m_device.unmapMemory(m_memory);
            m_mapped_data = nullptr;
        }

        return *this;
    }

    bool VulkanBuffer::is_mapped() const
    {
        return m_mapped_data != nullptr;
    }

    VulkanBuffer::~VulkanBuffer()
    {
        unmap_memory();
        DESTROY_CALL(destroyBuffer, m_buffer);
        DESTROY_CALL(freeMemory, m_memory);
    }

    VulkanStaticVertexBuffer& VulkanStaticVertexBuffer::create(const byte* data, size_t size)
    {
        m_buffer.create(size, data, vk::BufferUsageFlagBits::eVertexBuffer);
        return *this;
    }

    void VulkanStaticVertexBuffer::bind(byte stream_index, size_t stride, size_t offset)
    {
        RHI_VertexBuffer*& current = API->m_state.m_current_vertex_buffer[stream_index];
        if (current != this)
        {
            auto cmd = API->current_command_buffer();
            cmd->m_cmd.bindVertexBuffers(stream_index, m_buffer.m_buffer, {offset});
            cmd->add_object(this);
            current = this;
        }
    }

    void VulkanStaticVertexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        m_buffer.update(offset, data, size);
    }

    VulkanDynamicVertexBuffer& VulkanDynamicVertexBuffer::create(const byte* data, size_t size)
    {
        m_buffers.resize(API->m_framebuffers_count);
        for (auto& buffer : m_buffers)
        {
            buffer.create(size, data, vk::BufferUsageFlagBits::eVertexBuffer);
        }
        return *this;
    }

    void VulkanDynamicVertexBuffer::bind(byte stream_index, size_t stride, size_t offset)
    {
        RHI_VertexBuffer*& current_buffer = API->m_state.m_current_vertex_buffer[stream_index];
        if (current_buffer != this)
        {
            auto cmd = API->current_command_buffer();
            cmd->m_cmd.bindVertexBuffers(stream_index, current().m_buffer, {offset});
            cmd->add_object(this);
            current_buffer = this;
        }
    }

    void VulkanDynamicVertexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        current().update(offset, data, size);
    }

    VulkanBuffer& VulkanDynamicVertexBuffer::current()
    {
        return m_buffers[API->m_current_buffer];
    }

    VulkanIndexBuffer& VulkanIndexBuffer::create(const byte* data, size_t size, IndexBufferFormat format)
    {
        m_type = format == IndexBufferFormat::UInt32 ? vk::IndexType::eUint32 : vk::IndexType::eUint16;
        m_buffer.create(size, data, vk::BufferUsageFlagBits::eIndexBuffer);
        return *this;
    }

    void VulkanIndexBuffer::bind(size_t offset)
    {
        RHI_IndexBuffer*& current = API->m_state.m_current_index_buffer;
        if (current != this)
        {
            auto cmd = API->current_command_buffer();
            cmd->m_cmd.bindIndexBuffer(m_buffer.m_buffer, offset, m_type);
            cmd->add_object(this);
            current = this;
        }
    }

    void VulkanIndexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        m_buffer.update(offset, data, size);
    }

    VulkanDynamicIndexBuffer& VulkanDynamicIndexBuffer::create(const byte* data, size_t size, IndexBufferFormat format)
    {
        m_type = format == IndexBufferFormat::UInt32 ? vk::IndexType::eUint32 : vk::IndexType::eUint16;
        m_buffers.resize(API->m_framebuffers_count);

        for (auto& buffer : m_buffers)
        {
            buffer.create(size, data, vk::BufferUsageFlagBits::eIndexBuffer);
        }

        return *this;
    }

    void VulkanDynamicIndexBuffer::bind(size_t offset)
    {
        RHI_IndexBuffer*& current_buffer = API->m_state.m_current_index_buffer;
        if (current_buffer != this)
        {
            auto cmd = API->current_command_buffer();
            cmd->m_cmd.bindIndexBuffer(current().m_buffer, offset, m_type);
            cmd->add_object(this);
            current_buffer = this;
        }
    }
    void VulkanDynamicIndexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        current().update(offset, data, size);
    }

    VulkanBuffer& VulkanDynamicIndexBuffer::current()
    {
        return m_buffers[API->m_current_buffer];
    }

    VulkanSSBO& VulkanSSBO::create(const byte* data, size_t size)
    {
        m_buffer.create(size, data, vk::BufferUsageFlagBits::eStorageBuffer);
        return *this;
    }

    void VulkanSSBO::bind(BindLocation location)
    {
        if (API->m_state.m_pipeline)
        {
            API->m_state.m_pipeline->bind_ssbo(this, location);
        }
    }

    void VulkanSSBO::update(size_t offset, size_t size, const byte* data)
    {
        m_buffer.update(offset, data, size);
    }

    RHI_VertexBuffer* VulkanAPI::create_vertex_buffer(size_t size, const byte* data, RHIBufferType type)
    {
        if (type == RHIBufferType::Static)
            return &(new VulkanStaticVertexBuffer())->create(data, size);
        return &(new VulkanDynamicVertexBuffer())->create(data, size);
    }

    RHI_IndexBuffer* VulkanAPI::create_index_buffer(size_t size, const byte* data, IndexBufferFormat format, RHIBufferType type)
    {
        if (type == RHIBufferType::Static)
            return &(new VulkanIndexBuffer())->create(data, size, format);
        return &(new VulkanDynamicIndexBuffer())->create(data, size, format);
    }


    RHI_SSBO* VulkanAPI::create_ssbo(size_t size, const byte* data, RHIBufferType type)
    {
        return &(new VulkanSSBO())->create(data, size);
    }
}// namespace Engine