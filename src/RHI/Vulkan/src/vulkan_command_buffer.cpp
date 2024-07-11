#include <Graphics/rhi.hpp>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>

namespace Engine
{
    VulkanCommandBuffer::VulkanCommandBuffer()
    {
        vk::CommandBufferAllocateInfo alloc_info(API->m_command_pool, vk::CommandBufferLevel::ePrimary, 1);
        m_cmd = API->m_device.allocateCommandBuffers(alloc_info).front();
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other)
        : m_references(std::move(other.m_references)), m_cmd(std::move(other.m_cmd))
    {
        other.m_cmd = vk::CommandBuffer{};
    }

    VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other)
    {
        if (this == &other)
            return *this;

        m_references = std::move(other.m_references);
        m_cmd        = std::move(other.m_cmd);
        other.m_cmd  = vk::CommandBuffer{};
        return *this;
    }

    VulkanCommandBuffer& VulkanCommandBuffer::add_object(RHI_Object* object)
    {
        if (object)
        {
            m_references.push_back(object);
            object->add_reference();
        }
        return *this;
    }

    VulkanCommandBuffer& VulkanCommandBuffer::reset()
    {
        m_cmd.reset();
        release_references();
        return *this;
    }

    VulkanCommandBuffer& VulkanCommandBuffer::release_references()
    {
        for (RHI_Object* object : m_references)
        {
            object->release();
        }
        m_references.clear();
        return *this;
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        release_references();
        API->m_device.freeCommandBuffers(API->m_command_pool, m_cmd);
    }


}// namespace Engine
