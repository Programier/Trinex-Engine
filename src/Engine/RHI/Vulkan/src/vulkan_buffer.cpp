#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_state.hpp>
#include <vulkan_types.hpp>


namespace Engine
{
    VulkanBuffer& VulkanBuffer::create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlagBits type)
    {
        _M_size = size;
        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferDst | type, vk::MemoryPropertyFlagBits::eDeviceLocal,
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

        size                = std::min(size, _M_size - offset);
        MappedMemory memory = map_memory();
        std::memcpy(memory.data(), data, size);
        return *this;
    }

    MappedMemory VulkanBuffer::map_memory()
    {
        if (!_M_mapped_data)
        {
            _M_mapped_data = reinterpret_cast<byte*>(API->_M_device.mapMemory(_M_memory, 0, VK_WHOLE_SIZE));
        }

        return MappedMemory(_M_mapped_data, _M_size);
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
        API->_M_command_buffer->get().bindVertexBuffers(stream_index, _M_buffer._M_buffer, {offset});
    }

    MappedMemory VulkanVertexBuffer::map_buffer()
    {
        return _M_buffer.map_memory();
    }

    void VulkanVertexBuffer::unmap_buffer()
    {
        _M_buffer.unmap_memory();
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
        API->_M_command_buffer->get().bindIndexBuffer(_M_buffer._M_buffer, offset, _M_index_type);
    }

    MappedMemory VulkanIndexBuffer::map_buffer()
    {
        return _M_buffer.map_memory();
    }

    void VulkanIndexBuffer::unmap_buffer()
    {
        _M_buffer.unmap_memory();
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

    void VulkanSSBO::bind(BindingIndex binding, BindingIndex set)
    {
        throw EngineException("NOT IMPLEMENTED!");
    }

    MappedMemory VulkanSSBO::map_buffer()
    {
        return _M_buffer.map_memory();
    }

    void VulkanSSBO::unmap_buffer()
    {
        _M_buffer.unmap_memory();
    }

    void VulkanSSBO::update(size_t offset, size_t size, const byte* data)
    {
        _M_buffer.update(offset, data, size);
    }


    VulkanUniformBuffer::VulkanUniformBuffer(const byte* data, size_t size)
    {
        _M_buffer.reserve(MAIN_FRAMEBUFFERS_COUNT);
        for (int i = 0; i < MAIN_FRAMEBUFFERS_COUNT; i++)
        {
            VulkanBuffer* buffer = new VulkanBuffer();
            buffer->create(size, data, vk::BufferUsageFlagBits::eUniformBuffer);
            _M_buffer.push_back(buffer);
        }
    }

    VulkanBuffer* VulkanUniformBuffer::current_buffer()
    {
        return _M_buffer[API->_M_current_buffer];
    }

    void VulkanUniformBuffer::bind(BindingIndex binding, BindingIndex set)
    {}

    MappedMemory VulkanUniformBuffer::map_buffer()
    {
        return current_buffer()->map_memory();
    }

    void VulkanUniformBuffer::unmap_buffer()
    {
        current_buffer()->unmap_memory();
    }

    void VulkanUniformBuffer::update(size_t offset, size_t size, const byte* data)
    {
        current_buffer()->update(offset, data, size);
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        for (VulkanBuffer* buffer : _M_buffer)
        {
            delete buffer;
        }
    }

    RHI_VertexBuffer* VulkanAPI::create_vertex_buffer(size_t size, const byte* data)
    {
        return &(new VulkanVertexBuffer())->create(data, size);
    }

    RHI_IndexBuffer* VulkanAPI::create_index_buffer(size_t size, const byte* data, IndexBufferComponent component)
    {
        return &(new VulkanIndexBuffer())->create(data, size, component);
    }

    RHI_UniformBuffer* VulkanAPI::create_uniform_buffer(size_t size, const byte* data)
    {
        return new VulkanUniformBuffer(data, size);
    }

    RHI_SSBO* VulkanAPI::create_ssbo(size_t size, const byte* data)
    {
        return &(new VulkanSSBO())->create(data, size);
    }
}// namespace Engine
