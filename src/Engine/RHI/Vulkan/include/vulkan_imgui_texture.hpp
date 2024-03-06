#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan/vulkan.h>

namespace Engine
{
    struct VulkanImGuiTextureInterface : public RHI_ImGuiTexture {
        virtual VkDescriptorSet descriptor_set() = 0;

        FORCE_INLINE void* handle()
        {
            return this;
        }

        virtual ~VulkanImGuiTextureInterface()
        {}
    };

    struct VulkanImGuiTextureBasic : public VulkanImGuiTextureInterface {
        VkDescriptorSet m_set;

        FORCE_INLINE VulkanImGuiTextureBasic(VkDescriptorSet set) : m_set(set)
        {}

        FORCE_INLINE VkDescriptorSet descriptor_set() override
        {
            return m_set;
        }

        FORCE_INLINE void destroy_now() override
        {}
    };

    struct VulkanImGuiTexture : public VulkanImGuiTextureInterface {
        ImGuiContext* m_ctx                = nullptr;
        class Texture* m_texture           = nullptr;
        struct VulkanTexture* m_vk_texture = nullptr;

        VkDescriptorSet m_set = VK_NULL_HANDLE;

        FORCE_INLINE VulkanImGuiTexture(ImGuiContext* ctx, class Texture* texture) : m_ctx(ctx), m_texture(texture)
        {}

        VkDescriptorSet descriptor_set() override;
        void destroy_now() override;
        void destroy();
        ~VulkanImGuiTexture() override;
    };

}// namespace Engine
