#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <vulkan_api.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_transition_image_layout.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{

    void VulkanRenderTargetFrame::init(struct VulkanRenderTarget* owner, const RenderTarget* render_target,
                                       struct VulkanRenderPass* render_pass, Index frame)
    {
        destroy();
        m_owner = owner;

        m_attachments.resize(render_pass->attachments_count());

        Index index = 0;
        for (const Texture2D* color_binding : render_target->frame(frame)->color_attachments)
        {
            VulkanTexture* texture = color_binding->rhi_object<VulkanTexture>();

            trinex_check(texture, "Vulkan API: Cannot attach color texture: Texture is NULL");
            bool usage_check = texture->can_use_color_as_color_attachment();
            trinex_check(usage_check, "Vulkan API: Pixel type for color attachment must be RGBA");

            vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            m_attachments[index] = texture->get_image_view(range);
            ++index;
        }

        if (render_pass->m_has_depth_attachment)
        {
            const Texture2D* binding = render_target->frame(frame)->depth_stencil_attachment;
            VulkanTexture* texture   = binding->rhi_object<VulkanTexture>();
            trinex_check(texture, "Vulkan API: Cannot depth attach texture: Texture is NULL");

            bool check_status = texture->is_depth_stencil_image();
            trinex_check(check_status, "Vulkan API: Pixel type for depth attachment must be Depth* or Stencil*");

            vk::ImageSubresourceRange range(texture->aspect(), 0, 1, 0, 1);
            m_attachments[index] = texture->get_image_view(range);
        }
    }

    void VulkanRenderTargetFrame::post_init()
    {
        // Initialize framebuffer
        vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(),
                                                          m_owner->m_render_pass->m_render_pass, m_attachments,
                                                          m_owner->m_size.width, m_owner->m_size.height, 1);
        m_framebuffer = API->m_device.createFramebuffer(framebuffer_create_info);
    }


    void VulkanRenderTargetFrame::push_barriers(size_t count)
    {
        auto src_stage_mask  = vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eColorAttachmentOutput;
        auto dest_stage_mask = vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader |
                               vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader |
                               vk::PipelineStageFlagBits::eTransfer;


        std::vector<vk::MemoryBarrier> barriers(count);

        auto src_access_mask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        auto dst_access_mask = vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead |
                               vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite |
                               vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;

        for (auto& barrier : barriers)
        {
            barrier.setSrcAccessMask(src_access_mask);
            barrier.setDstAccessMask(dst_access_mask);
        }


        API->current_command_buffer().pipelineBarrier(src_stage_mask, dest_stage_mask, {}, barriers, {}, {});
    }

    void VulkanRenderTargetFrame::bind(RenderPass* render_pass)
    {
        if (API->m_state->m_framebuffer == this)
            return;


        if (API->m_state->m_framebuffer)
        {
            size_t count = API->m_state->m_framebuffer->m_attachments.size();
            API->m_state->m_framebuffer->unbind();
            push_barriers(count);
        }


        API->m_state->m_framebuffer = this;
        m_owner->m_render_pass_info.setFramebuffer(m_framebuffer);

        VulkanRenderPass* vulkan_render_pass = render_pass->rhi_object<VulkanRenderPass>();
        if (vulkan_render_pass == m_owner->m_render_pass)
        {
            API->current_command_buffer().beginRenderPass(m_owner->m_render_pass_info, vk::SubpassContents::eInline);
        }
        else
        {
            m_owner->m_render_pass_info.setRenderPass(vulkan_render_pass->m_render_pass);
            API->current_command_buffer().beginRenderPass(m_owner->m_render_pass_info, vk::SubpassContents::eInline);
            m_owner->m_render_pass_info.setRenderPass(m_owner->m_render_pass->m_render_pass);
        }

        if (m_owner->m_render_pass == API->m_main_render_pass)
        {
            API->m_state->m_is_image_rendered_to_swapchain = true;
        }

        update_viewport().update_scissors();
    }

    void VulkanRenderTargetFrame::unbind()
    {
        if (API->m_state->m_framebuffer == this)
        {
            API->current_command_buffer().endRenderPass();
            API->m_state->m_framebuffer = nullptr;
        }
    }

    VulkanRenderTargetFrame& VulkanRenderTargetFrame::update_viewport()
    {
        API->current_command_buffer().setViewport(0, m_owner->m_viewport);
        return *this;
    }

    VulkanRenderTargetFrame& VulkanRenderTargetFrame::update_scissors()
    {
        API->current_command_buffer().setScissor(0, m_owner->m_scissor);
        return *this;
    }

    void VulkanRenderTargetFrame::destroy()
    {
        DESTROY_CALL(destroyFramebuffer, m_framebuffer);

        if (!is_main_frame())
        {
            for (auto& image_view : m_attachments)
            {
                DESTROY_CALL(destroyImageView, image_view);
            }
        }
    }

    bool VulkanRenderTargetFrame::is_main_frame()
    {
        return false;
    }


    VulkanWindowRenderTargetFrame::VulkanWindowRenderTargetFrame()
    {}

    VulkanWindowRenderTargetFrame::~VulkanWindowRenderTargetFrame()
    {}


    bool VulkanWindowRenderTargetFrame::is_main_frame()
    {
        return true;
    }


    VulkanRenderTargetFrame::~VulkanRenderTargetFrame()
    {}

    VulkanRenderTarget& VulkanRenderTarget::init(const RenderTarget* render_target, VulkanRenderPass* render_pass)
    {
        m_render_pass = render_pass;
        m_size.width  = static_cast<uint32_t>(render_target->size.x);
        m_size.height = static_cast<uint32_t>(render_target->size.y);

        m_clear_values.resize(render_pass->attachments_count());

        for (Index index = 0, count = render_pass->attachments_count(); index < count; index++)
        {
            m_clear_values[index].color = vk::ClearColorValue(
                    Array<float, 4>({render_target->color_clear[index].x, render_target->color_clear[index].y,
                                     render_target->color_clear[index].z, render_target->color_clear[index].a}));
        }

        if (render_pass->m_has_depth_attachment)
        {
            m_clear_values.back().depthStencil = vk::ClearDepthStencilValue(render_target->depth_stencil_clear.depth,
                                                                             render_target->depth_stencil_clear.stencil);
        }


        // Initialize frames

        for (uint32_t i = 0; i < API->render_target_buffer_count(); i++)
        {
            VulkanRenderTargetFrame* frame = new VulkanRenderTargetFrame();
            frame->init(this, render_target, render_pass, i);
            m_frames.push_back(frame);
        }

        return post_init();
    }

    VulkanRenderTarget& VulkanRenderTarget::post_init()
    {
        m_viewport.x     = 0;
        m_viewport.width = static_cast<float>(m_size.width);

        if (is_main_render_target())
        {
            m_viewport.y      = static_cast<float>(m_size.height);
            m_viewport.height = -static_cast<float>(m_size.height);
        }
        else
        {
            m_viewport.y      = 0;
            m_viewport.height = static_cast<float>(m_size.height);
        }

        m_viewport.minDepth = 0.0f;
        m_viewport.maxDepth = 1.0f;

        m_scissor = vk::Rect2D({0, 0}, vk::Extent2D(m_size.width, m_size.height));

        // Initialize render_pass info
        m_render_pass_info.setRenderPass(m_render_pass->m_render_pass);
        m_render_pass_info.setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(m_size.width, m_size.height)));
        m_render_pass_info.setClearValues(m_clear_values);

        for (VulkanRenderTargetFrame* frame : m_frames)
        {
            frame->post_init();
        }

        return *this;
    }

    VulkanRenderTarget& VulkanRenderTarget::destroy()
    {
        for (VulkanRenderTargetFrame* frame : m_frames)
        {
            frame->destroy();
            delete frame;
        }

        return *this;
    }

    void VulkanRenderTarget::viewport(const ViewPort& viewport)
    {
        m_viewport.x        = viewport.pos.x;
        m_viewport.width    = viewport.size.x;
        m_viewport.minDepth = viewport.min_depth;
        m_viewport.maxDepth = viewport.max_depth;

        if (is_main_render_target())
        {
            // Revert Y
            m_viewport.height = -viewport.size.y;
            m_viewport.y      = m_size.height - viewport.pos.y;
        }
        else
        {
            m_viewport.y      = viewport.pos.y;
            m_viewport.height = viewport.size.y;
        }

        if (API->m_state->m_framebuffer == m_frames[buffer_index()])
            API->m_state->m_framebuffer->update_viewport();
    }

    void VulkanRenderTarget::scissor(const Scissor& scissor)
    {
        m_scissor.offset.x      = scissor.pos.x;
        m_scissor.extent.width  = scissor.size.x;
        m_scissor.extent.height = scissor.size.y;

        m_scissor.offset.y = scissor.pos.y;

        if (API->m_state->m_framebuffer == m_frames[buffer_index()])
            API->m_state->m_framebuffer->update_scissors();
    }


    Index VulkanRenderTarget::bind(RenderPass* render_pass)
    {
        auto index = buffer_index();
        m_frames[index]->bind(render_pass);
        return index;
    }

    void VulkanRenderTarget::clear_color(const ColorClearValue& color, byte layout)
    {
        byte layouts_count =
                static_cast<byte>(m_clear_values.size()) - static_cast<byte>(m_render_pass->m_has_depth_attachment);

        if (layout < layouts_count)
        {
            m_clear_values[layout].setColor(vk::ClearColorValue(Array<float, 4>({color.x, color.y, color.z, color.a})));
        }
        else
        {
            vulkan_debug_log("Vulkan API", "Incorrect layout index!");
        }
    }

    void VulkanRenderTarget::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        if (m_render_pass->m_has_depth_attachment)
        {
            m_clear_values.back().setDepthStencil(vk::ClearDepthStencilValue(value.depth, value.stencil));
        }
    }

    size_t VulkanRenderTarget::buffer_index() const
    {
        return API->m_current_buffer;
    }

    bool VulkanRenderTarget::is_main_render_target()
    {
        return false;
    }

    VulkanRenderTarget& VulkanRenderTarget::size(uint32_t width, uint32_t height)
    {
        m_size.width  = width;
        m_size.height = height;
        return *this;
    }

    VulkanRenderTarget::~VulkanRenderTarget()
    {
        destroy();
    }


    VulkanWindowRenderTarget& VulkanWindowRenderTarget::destroy()
    {
        for (VulkanRenderTargetFrame* frame : m_frames)
        {
            frame->destroy();
        }
        return *this;
    }

    size_t VulkanWindowRenderTarget::buffer_index() const
    {
        return m_viewport->m_buffer_index;
    }

    void VulkanWindowRenderTarget::resize_count(size_t new_count)
    {
        m_frames.resize(new_count);
        for (VulkanRenderTargetFrame*& frame : m_frames)
        {
            if (frame == nullptr)
                frame = new VulkanWindowRenderTargetFrame();
        }
    }

    static void init_main_framebuffer(VulkanViewport* viewport, VulkanRenderTargetFrame* frame, uint_t index)
    {
        frame->m_attachments.resize(1);
        frame->m_attachments[0] = viewport->m_image_views[index];
    }

    VulkanWindowRenderTarget& VulkanWindowRenderTarget::init(struct VulkanWindowViewport* viewport)
    {
        m_viewport = viewport;

        uint_t index   = 0;
        m_size.height = viewport->m_swapchain->extent.height;
        m_size.width  = viewport->m_swapchain->extent.width;
        m_render_pass = API->m_main_render_pass;

        for (VulkanRenderTargetFrame* frame : m_frames)
        {
            frame->m_owner = this;
            init_main_framebuffer(viewport, frame, index++);
        }

        post_init();

        return *this;
    }

    bool VulkanWindowRenderTarget::is_destroyable() const
    {
        return false;
    }

    bool VulkanWindowRenderTarget::is_main_render_target()
    {
        return true;
    }

    RHI_RenderTarget* VulkanAPI::create_render_target(const RenderTarget* render_target)
    {
        if (render_target->frames_count() != render_target_buffer_count())
            throw EngineException("Frames count is mismatch with API requirements");
        return &(new VulkanRenderTarget())->init(render_target, render_target->render_pass->rhi_object<VulkanRenderPass>());
    }
}// namespace Engine
