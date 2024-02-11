#include <Graphics/global_shader_parameters.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_state.hpp>
#include <vulkan_types.hpp>


namespace Engine
{
    VulkanBuffer& VulkanBuffer::create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type)
    {
        _M_size = size;
        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferDst | type, vk::MemoryPropertyFlagBits::eHostVisible,
                           _M_buffer, _M_memory);
        update(0, data, size);
        return *this;
    }

    VulkanBuffer& VulkanBuffer::update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size)
    {
        if (data == nullptr || offset >= _M_size)
            return *this;

        if (offset > _M_size)
        {
            return *this;
        }

        size         = std::min(size, _M_size - offset);
        void* memory = map_memory();
        std::memcpy(memory, data, size);
        return *this;
    }

    void* VulkanBuffer::map_memory()
    {
        if (!_M_mapped_data)
        {
            _M_mapped_data = reinterpret_cast<byte*>(API->_M_device.mapMemory(_M_memory, 0, VK_WHOLE_SIZE));
        }

        return _M_mapped_data;
    }

    VulkanBuffer& VulkanBuffer::unmap_memory()
    {
        if (_M_mapped_data)
        {
            API->_M_device.unmapMemory(_M_memory);
            _M_mapped_data = nullptr;
        }

        return *this;
    }

    bool VulkanBuffer::is_mapped() const
    {
        return _M_mapped_data != nullptr;
    }

    VulkanBuffer::~VulkanBuffer()
    {
        unmap_memory();
        DESTROY_CALL(destroyBuffer, _M_buffer);
        DESTROY_CALL(freeMemory, _M_memory);
    }


    VulkanVertexBuffer& VulkanVertexBuffer::create(const byte* data, size_t size)
    {
        _M_buffer.create(size, data, vk::BufferUsageFlagBits::eVertexBuffer);
        return *this;
    }

    void VulkanVertexBuffer::bind(byte stream_index, size_t offset)
    {
        VulkanVertexBuffer*& current = API->_M_state->_M_current_vertex_buffer[stream_index];
        if (current != this)
        {
            API->current_command_buffer().bindVertexBuffers(stream_index, _M_buffer._M_buffer, {offset});
            current = this;
        }
    }


    void VulkanVertexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        _M_buffer.update(offset, data, size);
    }


    VulkanIndexBuffer& VulkanIndexBuffer::create(const byte* data, size_t size, IndexBufferComponent component)
    {
        _M_index_type = get_type(component);
        _M_buffer.create(size, data, vk::BufferUsageFlagBits::eIndexBuffer);
        return *this;
    }

    void VulkanIndexBuffer::bind(size_t offset)
    {
        VulkanIndexBuffer*& current = API->_M_state->_M_current_index_buffer;
        if (current != this)
        {
            API->current_command_buffer().bindIndexBuffer(_M_buffer._M_buffer, offset, _M_index_type);
            current = this;
        }
    }

    void VulkanIndexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        _M_buffer.update(offset, data, size);
    }

    VulkanSSBO& VulkanSSBO::create(const byte* data, size_t size)
    {
        _M_buffer.create(size, data, vk::BufferUsageFlagBits::eStorageBuffer);
        return *this;
    }

    void VulkanSSBO::bind(BindLocation location)
    {
        if (API->_M_state->_M_pipeline)
        {
            API->_M_state->_M_pipeline->bind_ssbo(this, location);
        }
    }

    void VulkanSSBO::update(size_t offset, size_t size, const byte* data)
    {
        _M_buffer.update(offset, data, size);
    }

    RHI_VertexBuffer* VulkanAPI::create_vertex_buffer(size_t size, const byte* data)
    {
        return &(new VulkanVertexBuffer())->create(data, size);
    }

    RHI_IndexBuffer* VulkanAPI::create_index_buffer(size_t size, const byte* data, IndexBufferComponent component)
    {
        return &(new VulkanIndexBuffer())->create(data, size, component);
    }


    RHI_SSBO* VulkanAPI::create_ssbo(size_t size, const byte* data)
    {
        return &(new VulkanSSBO())->create(data, size);
    }
}// namespace Engine
