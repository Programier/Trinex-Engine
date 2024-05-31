#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>


namespace Engine
{
    struct VulkanRenderPass : public RHI_RenderPass {
        vk::RenderPass m_render_pass;
        vk::SubpassDescription m_subpass;
        vk::SubpassDependency m_dependency;

        Vector<vk::AttachmentDescription> m_attachment_descriptions;
        Vector<vk::AttachmentReference> m_color_attachment_references;
        vk::AttachmentReference m_depth_attachment_renference;
        bool m_has_depth_attachment = false;


        VulkanRenderPass& init(const RenderPass* render_pass);
        VulkanRenderPass& create_attachment_descriptions(const RenderPass* render_pass);
        VulkanRenderPass& create_attachment_references(const RenderPass* render_pass);
        VulkanRenderPass& create();
        uint_t attachments_count() const;
        uint_t color_attachments_count() const;

        VulkanRenderPass& destroy();
        ~VulkanRenderPass();
    };

    struct VulkanMainRenderPass : public VulkanRenderPass {
        bool is_destroyable() const override;
    };
}// namespace Engine
