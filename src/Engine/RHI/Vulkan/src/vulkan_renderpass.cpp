#include <Graphics/render_pass.hpp>
#include <Graphics/texture_2D.hpp>
#include <vulkan_api.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
    VulkanRenderPass& VulkanRenderPass::create_attachment_descriptions(const RenderPass* render_pass)
    {
        for (const ColorFormat& color_attachment : render_pass->color_attachments)
        {
            vk::Format format               = parse_engine_format(color_attachment);
            vk::AttachmentLoadOp clear_op   = vk::AttachmentLoadOp::eLoad;
            vk::ImageLayout input_layout_op = vk::ImageLayout::eShaderReadOnlyOptimal;

            vk::AttachmentDescription description = vk::AttachmentDescription(
                    vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1, clear_op,
                    vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    input_layout_op, vk::ImageLayout::eShaderReadOnlyOptimal);

            m_attachment_descriptions.push_back(description);
        }

        if (render_pass->depth_stencil_attachment != ColorFormat::Undefined)
        {
            m_has_depth_attachment                = true;
            vk::Format format                     = parse_engine_format(render_pass->depth_stencil_attachment);
            vk::AttachmentLoadOp clear_op         = vk::AttachmentLoadOp::eLoad;
            vk::ImageLayout input_layout_op       = vk::ImageLayout::eShaderReadOnlyOptimal;
            vk::AttachmentDescription description = vk::AttachmentDescription(
                    vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1, clear_op,
                    vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    input_layout_op, vk::ImageLayout::eShaderReadOnlyOptimal);

            m_attachment_descriptions.push_back(description);
        }
        return *this;
    }

    VulkanRenderPass& VulkanRenderPass::create_attachment_references(const RenderPass* render_pass)
    {
        for (int_t index = 0, count = render_pass->color_attachments.size(); index < count; ++index)
        {
            m_color_attachment_references.push_back(vk::AttachmentReference(index, vk::ImageLayout::eColorAttachmentOptimal));
        }

        if (render_pass->depth_stencil_attachment != ColorFormat::Undefined)
        {
            vk::ImageLayout layout;

            switch (render_pass->depth_stencil_attachment)
            {
                case ColorFormat::ShadowDepth:
                case ColorFormat::FilteredShadowDepth:
                case ColorFormat::D32F:
                    layout = vk::ImageLayout::eDepthAttachmentOptimal;
                    break;

                case ColorFormat::DepthStencil:
                    layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                    break;

                default:
                    layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                    break;
            }

            m_depth_attachment_renference = vk::AttachmentReference(render_pass->color_attachments.size(), layout);
        }
        return *this;
    }

    VulkanRenderPass& VulkanRenderPass::init(const RenderPass* render_pass)
    {
        return create_attachment_descriptions(render_pass).create_attachment_references(render_pass).create();
    }

    VulkanRenderPass& VulkanRenderPass::create()
    {
        m_subpass = vk::SubpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
                                           m_color_attachment_references, {},
                                           m_has_depth_attachment ? &m_depth_attachment_renference : nullptr);


        vk::PipelineStageFlags src_pipeline_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::PipelineStageFlags dst_pipeline_flags =
                vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader |
                vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eTransfer;

        vk::AccessFlags src_access_flags = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
        vk::AccessFlags dst_access_flags = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eTransferRead;

        if (m_has_depth_attachment)
        {
            src_pipeline_flags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
            src_access_flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        }

        m_dependency = vk::SubpassDependency(0, VK_SUBPASS_EXTERNAL, src_pipeline_flags, dst_pipeline_flags, src_access_flags,
                                             dst_access_flags, vk::DependencyFlagBits::eByRegion);

        m_render_pass = API->m_device.createRenderPass(
                vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), m_attachment_descriptions, m_subpass, m_dependency));

        return *this;
    }

    uint_t VulkanRenderPass::attachments_count() const
    {
        return m_attachment_descriptions.size();
    }

    uint_t VulkanRenderPass::color_attachments_count() const
    {
        uint_t attachments = attachments_count();

        if (m_has_depth_attachment)
        {
            --attachments;
        }
        return attachments;
    }

    VulkanRenderPass& VulkanRenderPass::destroy()
    {
        DESTROY_CALL(destroyRenderPass, m_render_pass);

        m_attachment_descriptions.clear();
        m_color_attachment_references.clear();
        m_has_depth_attachment = false;
        return *this;
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        destroy();
    }

    RHI_RenderPass* VulkanAPI::create_render_pass(const RenderPass* render_pass)
    {
        return &(new VulkanRenderPass())->init(render_pass);
    }


    bool VulkanMainRenderPass::is_destroyable() const
    {
        return false;
    }

    void VulkanAPI::create_render_pass(vk::Format format)
    {
        if (m_main_render_pass == nullptr)
        {
            m_main_render_pass                         = new VulkanMainRenderPass();
            m_main_render_pass->m_has_depth_attachment = false;

            m_main_render_pass->m_attachment_descriptions.push_back(vk::AttachmentDescription(
                    vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::ePresentSrcKHR));

            m_main_render_pass->m_color_attachment_references = {
                    vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
            };

            m_main_render_pass->create();
        }
    }


    RHI_RenderPass* VulkanAPI::window_render_pass(RenderPass* engine_render_pass)
    {
        engine_render_pass->color_attachments.resize(1);
        engine_render_pass->color_attachments[0] = ColorFormat::R8G8B8A8;
        return API->m_main_render_pass;
    }
}// namespace Engine
