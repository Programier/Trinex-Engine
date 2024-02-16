#include <Graphics/render_pass.hpp>
#include <Graphics/texture_2D.hpp>
#include <vulkan_api.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
    VulkanRenderPass& VulkanRenderPass::create_attachment_descriptions(const RenderPass* render_pass)
    {
        for (const RenderPass::Attachment& color_attachment : render_pass->color_attachments)
        {
            vk::Format format = parse_engine_format(color_attachment.format);

            if (ColorFormatInfo::info_of(color_attachment.format).aspect() != ColorFormatAspect::Color)
            {
                throw EngineException("Cannot use attachmet with non color format as color attachment");
            }

            vk::AttachmentLoadOp clear_op =
                    color_attachment.clear_on_bind ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;

            vk::AttachmentDescription description = vk::AttachmentDescription(
                    vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1, clear_op,
                    vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);


            _M_attachment_descriptions.push_back(description);
        }

        if (render_pass->has_depth_stancil)
        {
            _M_has_depth_attachment  = true;
            vk::Format format        = parse_engine_format(render_pass->depth_stencil_attachment.format);
            ColorFormatAspect aspect = ColorFormatInfo::info_of(render_pass->depth_stencil_attachment.format).aspect();
            if (aspect != ColorFormatAspect::Depth && aspect != ColorFormatAspect::Stencil &&
                aspect != ColorFormatAspect::DepthStencil)
            {
                throw EngineException("Cannot use attachmet with non depth-/stencil format as depth-/stencil attachment");
            }

            vk::AttachmentLoadOp clear_op = render_pass->depth_stencil_attachment.clear_on_bind ? vk::AttachmentLoadOp::eClear
                                                                                                : vk::AttachmentLoadOp::eLoad;

            vk::AttachmentDescription description = vk::AttachmentDescription(
                    vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1, clear_op,
                    vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);

            _M_attachment_descriptions.push_back(description);
        }
        return *this;
    }

    VulkanRenderPass& VulkanRenderPass::create_attachment_references(const RenderPass* render_pass)
    {
        for (int_t index = 0, count = render_pass->color_attachments.size(); index < count; ++index)
        {
            _M_color_attachment_references.push_back(vk::AttachmentReference(index, vk::ImageLayout::eColorAttachmentOptimal));
        }

        if (render_pass->has_depth_stancil)
        {
            ColorFormatAspect aspect = ColorFormatInfo::info_of(render_pass->depth_stencil_attachment.format).aspect();

            vk::ImageLayout layout;

            switch (aspect)
            {
                case ColorFormatAspect::Depth:
                    layout = vk::ImageLayout::eDepthAttachmentOptimal;
                    break;

                case ColorFormatAspect::Stencil:
                    layout = vk::ImageLayout::eStencilAttachmentOptimal;
                    break;

                case ColorFormatAspect::DepthStencil:
                    layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                    break;

                default:
                    layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                    break;
            }

            _M_depth_attachment_renference = vk::AttachmentReference(render_pass->color_attachments.size(), layout);
        }
        return *this;
    }

    VulkanRenderPass& VulkanRenderPass::init(const RenderPass* render_pass)
    {
        return create_attachment_descriptions(render_pass).create_attachment_references(render_pass).create();
    }

    VulkanRenderPass& VulkanRenderPass::create()
    {
        _M_subpass = vk::SubpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
                                            _M_color_attachment_references, {},
                                            _M_has_depth_attachment ? &_M_depth_attachment_renference : nullptr);

        vk::PipelineStageFlags pipeline_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::AccessFlags access_flags          = vk::AccessFlagBits::eColorAttachmentWrite;

        if (_M_has_depth_attachment)
        {
            pipeline_flags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
            access_flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        }

        _M_dependency =
                vk::SubpassDependency(0, 0, pipeline_flags, pipeline_flags, {}, access_flags, vk::DependencyFlagBits::eByRegion);

        _M_render_pass = API->_M_device.createRenderPass(
                vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), _M_attachment_descriptions, _M_subpass, _M_dependency));

        return *this;
    }

    uint_t VulkanRenderPass::attachments_count() const
    {
        return _M_attachment_descriptions.size();
    }

    VulkanRenderPass& VulkanRenderPass::destroy()
    {
        DESTROY_CALL(destroyRenderPass, _M_render_pass);

        _M_attachment_descriptions.clear();
        _M_color_attachment_references.clear();
        _M_has_depth_attachment = false;
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
        if (_M_main_render_pass == nullptr)
        {
            _M_main_render_pass                          = new VulkanMainRenderPass();
            _M_main_render_pass->_M_has_depth_attachment = false;

            _M_main_render_pass->_M_attachment_descriptions.push_back(vk::AttachmentDescription(
                    vk::AttachmentDescriptionFlags(), format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR));

            _M_main_render_pass->_M_color_attachment_references = {
                    vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
            };

            _M_main_render_pass->create();
        }
    }


    RHI_RenderPass* VulkanAPI::window_render_pass(RenderPass* engine_render_pass)
    {
        engine_render_pass->color_attachments.resize(1);
        engine_render_pass->color_attachments[0].clear_on_bind = true;
        engine_render_pass->color_attachments[0].format =
                to_engine_format(API->_M_main_render_pass->_M_attachment_descriptions[0].format);
        return API->_M_main_render_pass;
    }
}// namespace Engine
