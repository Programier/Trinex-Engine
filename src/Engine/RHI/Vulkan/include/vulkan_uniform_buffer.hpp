#pragma once
#include <Core/memory.hpp>
#include <Graphics/shader_parameters.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
    struct UniformBufferPoolBase {
        struct BufferEntry {
            vk::Buffer buffer;
            vk::DeviceMemory memory;
            size_t size;
            byte* mapped;
        };

        Vector<BufferEntry> buffers;

    protected:
        UniformBufferPoolBase& allocate_new(size_t size);
        UniformBufferPoolBase& update(size_t index, const void* data, size_t size, size_t offset);

        ~UniformBufferPoolBase();
    };

    template<size_t pool_size>
    struct UniformBufferPool : public UniformBufferPoolBase {
        UniformBufferPool& allocate_new(size_t size = pool_size)
        {
            UniformBufferPoolBase::allocate_new(glm::max(size, pool_size));
            return *this;
        }
    };

    struct GlobalUniformBufferPool : public UniformBufferPool<sizeof(GlobalShaderParameters)> {
        int64_t index = -1;

        void push(const GlobalShaderParameters* params = nullptr);
        void pop();

        void bind();
        void reset();
    };

    struct LocalUniformBufferPool : public UniformBufferPool<4096> {
        Vector<byte> shadow_data;
        size_t shadow_data_size = 0;
        size_t index            = 0;
        size_t used_data        = 0;

        LocalUniformBufferPool();

        void update(const void* data, size_t size, size_t offset);
        void bind();
        void reset();
    };

    struct VulkanUniformBuffer {
        GlobalUniformBufferPool global_pool;
        LocalUniformBufferPool local_pool;

        void reset();
        void bind();
    };
}// namespace Engine
