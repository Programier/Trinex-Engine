#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>


namespace Engine
{
    struct VulkanRenderPass : public RHI_RenderPass
    {
        vk::RenderPass _M_render_pass;
        vk::SubpassDescription _M_subpass;
        vk::SubpassDependency _M_dependency;

        Vector<vk::AttachmentDescription> _M_attachment_descriptions;
        Vector<vk::AttachmentReference> _M_color_attachment_references;
        vk::AttachmentReference _M_depth_attachment_renference;
        bool _M_has_depth_attachment = false;


        VulkanRenderPass& init(const RenderPass* render_pass);
        VulkanRenderPass& create_attachment_descriptions(const RenderPass* render_pass);
        VulkanRenderPass& create_attachment_references(const RenderPass* render_pass);
        VulkanRenderPass& create();
        uint_t attachments_count() const;

        VulkanRenderPass& destroy();
        ~VulkanRenderPass();
    };
}// namespace Engine
