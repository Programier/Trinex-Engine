#include <vulkan_api.hpp>
#include <vulkan_mesh.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
    void* VulkanMesh::get_instance_data()
    {
        return this;
    }

    static void create_mesh_buffer(vk::DeviceSize size, void* data, vk::Buffer& out_buffer,
                                   vk::DeviceMemory& out_buffer_memory, vk::BufferUsageFlagBits type)
    {
        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;

        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                           staging_buffer, staging_buffer_memory);

        void* vulkan_data = nullptr;
        vulkan_data = API->_M_device.mapMemory(staging_buffer_memory, 0, size, {});

        std::memcpy(vulkan_data, data, size);
        API->_M_device.unmapMemory(staging_buffer_memory);


        API->create_buffer(size, vk::BufferUsageFlagBits::eTransferDst | type, vk::MemoryPropertyFlagBits::eDeviceLocal,
                           out_buffer, out_buffer_memory);

        API->copy_buffer(staging_buffer, out_buffer, size);

        API->_M_device.destroyBuffer(staging_buffer);
        API->_M_device.freeMemory(staging_buffer_memory);
    }

    VulkanMesh& VulkanMesh::data(size_t size, DrawMode mode, void* data)
    {
        create_mesh_buffer(size, data, _M_vertex_buffer, _M_vertex_buffer_memory,
                           vk::BufferUsageFlagBits::eVertexBuffer);
        return *this;
    }

    VulkanMesh& VulkanMesh::index_buffer(size_t size, IndexBufferComponent type, void* data)
    {
        _M_index_type = _M_index_types.at(type);
        create_mesh_buffer(size, data, _M_index_buffer, _M_index_buffer_memory, vk::BufferUsageFlagBits::eIndexBuffer);
        return *this;
    }

    VulkanMesh& VulkanMesh::draw(Primitive primitive, size_t vertices, size_t offset)
    {
        API->_M_current_command_buffer->bindVertexBuffers(0, _M_vertex_buffer, {0});
        API->_M_current_command_buffer->bindIndexBuffer(_M_index_buffer, 0, _M_index_type);

        //API->_M_current_command_buffer->setPrimitiveTopology(_M_primitive_topologies.at(primitive));

        API->_M_current_command_buffer->drawIndexed(vertices, 1, offset, 0, 0);
        return *this;
    }

    VulkanMesh& VulkanMesh::clean()
    {

        if (_M_index_buffer)
        {
            API->_M_device.destroyBuffer(_M_index_buffer);
        }

        if (_M_index_buffer_memory)
        {
            API->_M_device.freeMemory(_M_index_buffer_memory);
        }

        if (_M_vertex_buffer)
        {
            API->_M_device.destroyBuffer(_M_vertex_buffer);
        }

        if (_M_vertex_buffer_memory)
        {
            API->_M_device.freeMemory(_M_vertex_buffer_memory);
        }

        return *this;
    }

    VulkanMesh::~VulkanMesh()
    {
        clean();
    }

}// namespace Engine
