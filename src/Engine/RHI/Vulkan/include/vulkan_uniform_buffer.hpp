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
            void* mapped;
        };

        Vector<BufferEntry> buffers;

    protected:
        UniformBufferPoolBase& allocate_new(size_t size);
        UniformBufferPoolBase& update(size_t index, void* data, size_t size, size_t offset);

        ~UniformBufferPoolBase();
    };

    template<size_t pool_size>
    struct UniformBufferPool : public UniformBufferPoolBase {
        UniformBufferPool& allocate_new()
        {
            UniformBufferPoolBase::allocate_new(pool_size);
            return *this;
        }
    };

    struct GlobalUniformBufferPool : public UniformBufferPool<align_memory(sizeof(GlobalShaderParameters), 256)> {
        int64_t index = -1;

        void push(GlobalShaderParameters* params = nullptr);
        void update(void* data, size_t size, size_t offset);
        void pop();

        void bind();
        void reset();
    };

    struct LocalUniformBufferPool : public UniformBufferPool<4096> {
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
