#include <Core/default_resources.hpp>
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
        VulkanTexture* texture = m_texture->rhi_object<VulkanTexture>();
        VulkanSampler* sampler = DefaultResources::default_sampler->rhi_object<VulkanSampler>();

        if (texture == nullptr || sampler == nullptr)
        {
            throw EngineException("Cannot initialize imgui texture without sampler or texture");
        }

        if (m_vk_texture != texture)
        {
            destroy();
            m_set        = ImGui_ImplVulkan_AddTexture(sampler->m_sampler, texture->m_image_view,
                                                       static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
            m_vk_texture = texture;
        }

        return m_set;
    }

    void VulkanImGuiTexture::destroy_now()
    {
        destroy();
    }

    void VulkanImGuiTexture::destroy()
    {
        if (m_set != VK_NULL_HANDLE)
        {
            ImGui::SetCurrentContext(m_ctx);
            ImGui_ImplVulkan_RemoveTexture(m_set);
            m_set = VK_NULL_HANDLE;
        }
    }

    VulkanImGuiTexture::~VulkanImGuiTexture()
    {
        destroy();
    }

    RHI_ImGuiTexture* VulkanAPI::imgui_create_texture(ImGuiContext* ctx, Texture* texture)
    {
        ImGui::SetCurrentContext(ctx);
        trinex_check(ctx && texture, "Cannot create ImGui Texture from invalid data!");
        return new VulkanImGuiTexture(ctx, texture);
    }
}// namespace Engine
