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
        VkDescriptorSet _M_set;

        FORCE_INLINE VulkanImGuiTextureBasic(VkDescriptorSet set) : _M_set(set)
        {}

        FORCE_INLINE VkDescriptorSet descriptor_set() override
        {
            return _M_set;
        }

        FORCE_INLINE void destroy_now() override
        {}
    };

    struct VulkanImGuiTexture : public VulkanImGuiTextureInterface {
        ImGuiContext* _M_ctx                = nullptr;
        class Texture* _M_texture           = nullptr;
        class Sampler* _M_sampler           = nullptr;
        struct VulkanTexture* _M_vk_texture = nullptr;
        struct VulkanSampler* _M_vk_sampler = nullptr;

        VkDescriptorSet _M_set = VK_NULL_HANDLE;

        FORCE_INLINE VulkanImGuiTexture(ImGuiContext* ctx, class Texture* texture, class Sampler* sampler)
            : _M_ctx(ctx), _M_texture(texture), _M_sampler(sampler)
        {}

        VkDescriptorSet descriptor_set() override;
        void destroy_now() override;
        void destroy();
        ~VulkanImGuiTexture() override;
    };

}// namespace Engine
