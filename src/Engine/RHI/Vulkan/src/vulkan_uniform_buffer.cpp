#include <vulkan_api.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_state.hpp>
#include <vulkan_uniform_buffer.hpp>

namespace Engine
{
    UniformBufferPoolBase& UniformBufferPoolBase::allocate_new(size_t size)
    {
        buffers.emplace_back();
        auto& buffer = buffers.back();
        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
                           vk::MemoryPropertyFlagBits::eHostVisible, buffer.buffer, buffer.memory);

        buffer.size   = size;
        buffer.mapped = API->_M_device.mapMemory(buffer.memory, 0, size);
        return *this;
    }

    UniformBufferPoolBase& UniformBufferPoolBase::update(size_t index, void* data, size_t size, size_t offset)
    {
        if (index >= buffers.size())
            return *this;

        auto& buffer = buffers[index];
        if (offset >= buffer.size)
            return *this;

        size = glm::min(size, buffer.size - offset);
        std::memcpy(buffer.mapped, data, size);
        return *this;
    }

    UniformBufferPoolBase::~UniformBufferPoolBase()
    {
        for (BufferEntry& entry : buffers)
        {
            API->_M_device.unmapMemory(entry.memory);
            DESTROY_CALL(destroyBuffer, entry.buffer);
            DESTROY_CALL(freeMemory, entry.memory);
        }

        buffers.clear();
    }


    void GlobalUniformBufferPool::push(GlobalShaderParameters* params)
    {
        ++index;
        if (index >= static_cast<int64_t>(buffers.size()))
            allocate_new();

        if (params)
        {
            update(params, sizeof(GlobalShaderParameters), 0);
        }
    }

    void GlobalUniformBufferPool::update(void* data, size_t size, size_t offset)
    {
        UniformBufferPoolBase::update(index, data, size, offset);
    }

    void GlobalUniformBufferPool::pop()
    {
        --index;
    }

    void GlobalUniformBufferPool::bind()
    {
        if (index >= 0)
        {
            API->_M_state->_M_pipeline->bind_uniform_buffer(buffers[index].buffer, 0, sizeof(GlobalShaderParameters),
                                                            BindLocation(0, 0));
        }
    }

    void GlobalUniformBufferPool::reset()
    {
        index = -1;
    }

    void LocalUniformBufferPool::bind()
    {}

    void LocalUniformBufferPool::reset()
    {}

    void VulkanUniformBuffer::reset()
    {
        global_pool.reset();
        local_pool.reset();
    }

    void VulkanUniformBuffer::bind()
    {
        global_pool.bind();
        local_pool.bind();
    }


    VulkanAPI& VulkanAPI::push_global_params(GlobalShaderParameters* params)
    {
        API->uniform_buffer()->global_pool.push(params);
        return *this;
    }

    VulkanAPI& VulkanAPI::update_global_params(void* data, size_t size, size_t offset)
    {
        API->uniform_buffer()->global_pool.update(data, size, offset);
        return *this;
    }

    VulkanAPI& VulkanAPI::pop_global_params()
    {
        API->uniform_buffer()->global_pool.pop();
        return *this;
    }
}// namespace Engine
