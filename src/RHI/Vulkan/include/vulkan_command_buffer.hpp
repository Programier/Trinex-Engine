#pragma once
#include <Core/engine_types.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{
    struct RHI_Object;

    struct VulkanCommandBuffer final {
        Vector<RHI_Object*> m_references;
        vk::CommandBuffer m_cmd;

        VulkanCommandBuffer();
        VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
        VulkanCommandBuffer(VulkanCommandBuffer&&);

        VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
        VulkanCommandBuffer& operator=(VulkanCommandBuffer&&);

        VulkanCommandBuffer& add_object(RHI_Object* object);
        VulkanCommandBuffer& reset();
        VulkanCommandBuffer& release_references();

        virtual ~VulkanCommandBuffer();
    };
}// namespace Engine
