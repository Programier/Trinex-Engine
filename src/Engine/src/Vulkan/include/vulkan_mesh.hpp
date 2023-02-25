#pragma once
#include <vulkan_object.hpp>

namespace Engine
{
    struct VulkanMesh : VulkanObject
    {
        vk::Buffer _M_vertex_buffer;
        vk::Buffer _M_index_buffer;
        vk::DeviceMemory _M_vertex_buffer_memory;
        vk::DeviceMemory _M_index_buffer_memory;
        vk::IndexType _M_index_type;

        void* get_instance_data() override;

        VulkanMesh& data(size_t size, DrawMode mode, void* data);
        VulkanMesh& index_buffer(size_t size, IndexBufferComponent type, void* data);
        VulkanMesh& draw(Primitive primitive, size_t vertices, size_t offset);
        VulkanMesh& clean();
        ~VulkanMesh();
    };
}
