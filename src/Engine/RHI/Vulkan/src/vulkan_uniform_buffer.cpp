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
        buffer.mapped = reinterpret_cast<byte*>(API->_M_device.mapMemory(buffer.memory, 0, size));
        return *this;
    }

    UniformBufferPoolBase& UniformBufferPoolBase::update(size_t index, const void* data, size_t size, size_t offset)
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

    void GlobalUniformBufferPool::push(const GlobalShaderParameters* params)
    {
        ++index;
        if (index >= static_cast<int64_t>(buffers.size()))
            allocate_new();
        UniformBufferPoolBase::update(index, params, sizeof(GlobalShaderParameters), 0);
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

    LocalUniformBufferPool::LocalUniformBufferPool()
    {
        allocate_new();
    }

    void LocalUniformBufferPool::bind()
    {
        if (shadow_data_size == 0)
            return;

        if (buffers[index].size < used_data + shadow_data_size)
        {
            ++index;
            used_data = 0;

            if (buffers.size() <= index)
            {
                allocate_new(shadow_data_size);
            }
        }

        auto& current_buffer = buffers[index];
        std::memcpy(current_buffer.mapped + used_data, shadow_data.data(), shadow_data_size);
        static BindLocation local_params_location = {1, 0};
        API->_M_state->_M_pipeline->bind_uniform_buffer(current_buffer.buffer, used_data, shadow_data_size,
                                                        local_params_location);
        used_data = align_memory(used_data + shadow_data_size, API->_M_properties.limits.minUniformBufferOffsetAlignment);
    }

    void LocalUniformBufferPool::update(const void* data, size_t size, size_t offset)
    {
        shadow_data_size = glm::max(size + offset, shadow_data_size);

        if (shadow_data.size() < shadow_data_size)
        {
            shadow_data.resize(shadow_data_size);
        }

        std::memcpy(shadow_data.data() + offset, data, size);
    }

    void LocalUniformBufferPool::reset()
    {
        shadow_data_size = 0;
        index            = 0;
    }

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


    VulkanAPI& VulkanAPI::push_global_params(const GlobalShaderParameters& params)
    {
        API->uniform_buffer()->global_pool.push(&params);
        return *this;
    }

    VulkanAPI& VulkanAPI::pop_global_params()
    {
        API->uniform_buffer()->global_pool.pop();
        return *this;
    }

    VulkanAPI& VulkanAPI::update_local_parameter(const void* data, size_t size, size_t offset)
    {
        API->uniform_buffer()->local_pool.update(data, size, offset);
        return *this;
    }
}// namespace Engine