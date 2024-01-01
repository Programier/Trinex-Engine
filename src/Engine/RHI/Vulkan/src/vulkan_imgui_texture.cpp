#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <imgui_impl_vulkan.h>
#include <vulkan_api.hpp>
#include <vulkan_imgui_texture.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_texture.hpp>


namespace Engine
{
    VkDescriptorSet VulkanImGuiTexture::descriptor_set()
    {
        VulkanTexture* texture = _M_texture->rhi_object<VulkanTexture>();
        VulkanSampler* sampler = _M_sampler->rhi_object<VulkanSampler>();

        if (texture == nullptr || sampler == nullptr)
        {
            throw EngineException("Cannot initialize imgui texture without sampler or texture");
        }

        if (_M_vk_sampler != sampler || _M_vk_texture != texture)
        {
            destroy();
            _M_set        = ImGui_ImplVulkan_AddTexture(sampler->_M_sampler, texture->_M_image_view,
                                                        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
            _M_vk_sampler = sampler;
            _M_vk_texture = texture;
        }

        return _M_set;
    }

    void VulkanImGuiTexture::destroy_now()
    {
        destroy();
    }

    void VulkanImGuiTexture::destroy()
    {
        if (_M_set != VK_NULL_HANDLE)
        {
            ImGui::SetCurrentContext(_M_ctx);
            ImGui_ImplVulkan_RemoveTexture(_M_set);
            _M_set = VK_NULL_HANDLE;
        }
    }

    VulkanImGuiTexture::~VulkanImGuiTexture()
    {
        destroy();
    }

    RHI_ImGuiTexture* VulkanAPI::imgui_create_texture(ImGuiContext* ctx, Texture* texture, Sampler* sampler)
    {
        ImGui::SetCurrentContext(ctx);
        trinex_check(ctx && texture && sampler, "Cannot create ImGui Texture from invalid data!");
        return new VulkanImGuiTexture(ctx, texture, sampler);
    }
}// namespace Engine
