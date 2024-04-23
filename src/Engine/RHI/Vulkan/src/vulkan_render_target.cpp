#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/render_target_texture.hpp>
#include <vulkan_api.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_transition_image_layout.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{
    static void push_barriers(size_t count)
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

    void VulkanRenderTargetState::init(const RenderTarget* render_target, VulkanRenderPass* render_pass)
    {
        m_render_pass = render_pass;
        m_size.width  = static_cast<uint32_t>(render_target->size.x);
        m_size.height = static_cast<uint32_t>(render_target->size.y);
        m_clear_values.resize(render_pass->attachments_count());

        for (Index index = 0, count = render_pass->attachments_count(); index < count; index++)
        {
            m_clear_values[index].color = vk::ClearColorValue(Array<float, 4>(
                    {render_target->color_attachments[index].color_clear.x, render_target->color_attachments[index].color_clear.y,
                     render_target->color_attachments[index].color_clear.z,
                     render_target->color_attachments[index].color_clear.a}));
        }

        if (render_pass->m_has_depth_attachment)
        {
            m_clear_values.back().depthStencil =
                    vk::ClearDepthStencilValue(render_target->depth_stencil_attachment.depth_stencil_clear.depth,
                                               render_target->depth_stencil_attachment.depth_stencil_clear.stencil);
        }
        post_init();
    }

    void VulkanRenderTargetState::post_init()
    {
        m_viewport.x     = 0;
        m_viewport.width = static_cast<float>(m_size.width);

        if (is_main_render_target_state())
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

        m_render_pass_info.setRenderPass(m_render_pass->m_render_pass);
        m_render_pass_info.setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(m_size.width, m_size.height)));
        m_render_pass_info.setClearValues(m_clear_values);
    }

    bool VulkanRenderTargetState::is_main_render_target_state()
    {
        return false;
    }

    bool VulkanMainRenderTargetState::is_main_render_target_state()
    {
        return true;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::init(const RenderTarget* render_target, VulkanRenderPass* render_pass)
    {
        m_attachments.resize(render_pass->attachments_count());

        Index index = 0;
        for (auto& attachment : render_target->color_attachments)
        {
            const Texture2D* color_binding = attachment.texture.ptr();
            VulkanTexture* texture         = color_binding->rhi_object<VulkanTexture>();

            trinex_check(texture, "Vulkan API: Cannot attach color texture: Texture is NULL");
            bool usage_check = texture->can_use_color_as_color_attachment();
            trinex_check(usage_check, "Vulkan API: Pixel type for color attachment must be RGBA");

            vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            m_attachments[index] = texture->get_image_view(range);
            ++index;
        }

        if (render_pass->m_has_depth_attachment)
        {
            const Texture2D* binding = render_target->depth_stencil_attachment.texture.ptr();
            VulkanTexture* texture   = binding->rhi_object<VulkanTexture>();
            trinex_check(texture, "Vulkan API: Cannot depth attach texture: Texture is NULL");

            bool check_status = texture->is_depth_stencil_image();
            trinex_check(check_status, "Vulkan API: Pixel type for depth attachment must be Depth* or Stencil*");

            vk::ImageSubresourceRange range(texture->aspect(), 0, 1, 0, 1);
            m_attachments[index] = texture->get_image_view(range);
        }


        return post_init();
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::post_init()
    {
        vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(), m_state->m_render_pass->m_render_pass,
                                                          m_attachments, m_state->m_size.width, m_state->m_size.height, 1);
        m_framebuffer = API->m_device.createFramebuffer(framebuffer_create_info);

        return *this;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::destroy(bool)
    {
        DESTROY_CALL(destroyFramebuffer, m_framebuffer);
        return *this;
    }

    void VulkanRenderTargetBase::viewport(const ViewPort& viewport)
    {
        m_state->m_viewport.x        = viewport.pos.x;
        m_state->m_viewport.width    = viewport.size.x;
        m_state->m_viewport.minDepth = viewport.min_depth;
        m_state->m_viewport.maxDepth = viewport.max_depth;

        if (is_main_render_target())
        {
            // Revert Y
            m_state->m_viewport.height = -viewport.size.y;
            m_state->m_viewport.y      = m_state->m_size.height - viewport.pos.y;
        }
        else
        {
            m_state->m_viewport.y      = viewport.pos.y;
            m_state->m_viewport.height = viewport.size.y;
        }

        if (API->m_state->m_render_target.m_render_target == this)
            API->m_state->m_render_target.m_render_target->update_viewport();
    }

    void VulkanRenderTargetBase::scissor(const Scissor& scissor)
    {
        m_state->m_scissor.offset.x      = scissor.pos.x;
        m_state->m_scissor.extent.width  = scissor.size.x;
        m_state->m_scissor.extent.height = scissor.size.y;

        m_state->m_scissor.offset.y = scissor.pos.y;

        if (API->m_state->m_render_target.m_render_target == this)
            API->m_state->m_render_target.m_render_target->update_scissors();
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::update_viewport()
    {
        API->current_command_buffer().setViewport(0, m_state->m_viewport);
        return *this;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::update_scissors()
    {
        API->current_command_buffer().setScissor(0, m_state->m_scissor);
        return *this;
    }


    void VulkanRenderTargetBase::bind(RenderPass* render_pass)
    {
        VulkanRenderPass* vulkan_render_pass = render_pass->rhi_object<VulkanRenderPass>();

        if (API->m_state->m_render_target.m_render_target == this &&
            API->m_state->m_render_target.m_render_pass == vulkan_render_pass)
        {
            return;
        }

        if (API->m_state->m_render_target.m_render_target)
        {
            size_t count = API->m_state->m_render_target.m_render_target->m_attachments.size();
            API->m_state->m_render_target.m_render_target->unbind();
            push_barriers(count);
        }

        API->m_state->m_render_target.m_render_target = this;
        API->m_state->m_render_target.m_render_pass   = vulkan_render_pass;

        m_state->m_render_pass_info.setFramebuffer(m_framebuffer);


        if (vulkan_render_pass == m_state->m_render_pass)
        {
            API->current_command_buffer().beginRenderPass(m_state->m_render_pass_info, vk::SubpassContents::eInline);
        }
        else
        {
            m_state->m_render_pass_info.setRenderPass(vulkan_render_pass->m_render_pass);
            API->current_command_buffer().beginRenderPass(m_state->m_render_pass_info, vk::SubpassContents::eInline);
            m_state->m_render_pass_info.setRenderPass(m_state->m_render_pass->m_render_pass);
        }

        if (m_state->m_render_pass == API->m_main_render_pass)
        {
            API->m_state->m_is_image_rendered_to_swapchain = true;
        }

        update_viewport().update_scissors();
        return;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::unbind(VulkanRenderPass* next_render_pass)
    {
        if (API->m_state->m_render_target.m_render_target == this)
        {
            API->current_command_buffer().endRenderPass();
            API->m_state->m_render_target.m_render_target = nullptr;
            API->m_state->m_render_target.m_render_pass   = nullptr;
        }
        return *this;
    }

    void VulkanRenderTargetBase::clear_color(const ColorClearValue& color, byte layout)
    {
        byte layouts_count = static_cast<byte>(m_state->m_clear_values.size()) -
                             static_cast<byte>(m_state->m_render_pass->m_has_depth_attachment);

        if (layout < layouts_count)
        {
            m_state->m_clear_values[layout].setColor(vk::ClearColorValue(Array<float, 4>({color.x, color.y, color.z, color.a})));
        }
        else
        {
            vulkan_debug_log("Vulkan API", "Incorrect layout index!");
        }
    }

    void VulkanRenderTargetBase::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        if (m_state->m_render_pass->m_has_depth_attachment)
        {
            m_state->m_clear_values.back().setDepthStencil(vk::ClearDepthStencilValue(value.depth, value.stencil));
        }
    }

    bool VulkanRenderTargetBase::is_main_render_target()
    {
        return false;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::size(uint32_t width, uint32_t height)
    {
        m_state->m_size.width  = width;
        m_state->m_size.height = height;
        return *this;
    }

    VulkanRenderTargetBase::~VulkanRenderTargetBase()
    {
        destroy(true);
    }

    bool VulkanWindowRenderTargetFrame::is_main_render_target()
    {
        return true;
    }

    VulkanRenderTarget::VulkanRenderTarget()
    {
        m_state = &state;
    }

    VulkanRenderTarget& VulkanRenderTarget::init(const RenderTarget* target, VulkanRenderPass* render_pass)
    {
        state.init(target, render_pass);
        VulkanRenderTargetBase::init(target, render_pass);
        return *this;
    }

    VulkanRenderTarget& VulkanRenderTarget::destroy(bool called_by_destructor)
    {
        if (!called_by_destructor)
        {
            VulkanRenderTargetBase::destroy();
        }

        for (auto& image_view : m_attachments)
        {
            DESTROY_CALL(destroyImageView, image_view);
        }

        return *this;
    }

    VulkanRenderTarget::~VulkanRenderTarget()
    {
        destroy(true);
    }


    VulkanWindowRenderTarget& VulkanWindowRenderTarget::destroy()
    {
        for (VulkanWindowRenderTargetFrame* frame : m_frames)
        {
            frame->destroy();
        }
        return *this;
    }

    void VulkanWindowRenderTarget::resize_count(size_t new_count)
    {
        m_frames.resize(new_count);
        for (VulkanWindowRenderTargetFrame*& frame : m_frames)
        {
            if (frame == nullptr)
                frame = new VulkanWindowRenderTargetFrame();
        }
    }

    static void init_main_framebuffer(VulkanViewport* viewport, VulkanWindowRenderTargetFrame* frame, uint_t index)
    {
        frame->m_attachments.resize(1);
        frame->m_attachments[0] = viewport->m_image_views[index];
    }

    VulkanWindowRenderTarget& VulkanWindowRenderTarget::init(struct VulkanWindowViewport* viewport)
    {
        m_viewport = viewport;

        uint_t index        = 0;
        state.m_size.height = viewport->m_swapchain->extent.height;
        state.m_size.width  = viewport->m_swapchain->extent.width;
        state.m_render_pass = API->m_main_render_pass;
        state.post_init();

        for (VulkanWindowRenderTargetFrame* frame : m_frames)
        {
            frame->m_state = &state;
            init_main_framebuffer(viewport, frame, index++);
            frame->post_init();
        }
        return *this;
    }

    bool VulkanWindowRenderTarget::is_destroyable() const
    {
        return false;
    }

    VulkanWindowRenderTargetFrame* VulkanWindowRenderTarget::frame()
    {
        return m_frames[m_viewport->m_buffer_index];
    }

    void VulkanWindowRenderTarget::bind(RenderPass* render_pass)
    {
        frame()->bind(render_pass);
    }

    void VulkanWindowRenderTarget::viewport(const ViewPort& viewport)
    {
        frame()->viewport(viewport);
    }

    void VulkanWindowRenderTarget::scissor(const Scissor& scissor)
    {
        frame()->scissor(scissor);
    }

    void VulkanWindowRenderTarget::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        frame()->clear_depth_stencil(value);
    }

    void VulkanWindowRenderTarget::clear_color(const ColorClearValue& color, byte layout)
    {
        frame()->clear_color(color, layout);
    }

    VulkanWindowRenderTarget::~VulkanWindowRenderTarget()
    {
        for (VulkanWindowRenderTargetFrame* frame : m_frames)
        {
            delete frame;
        }
        m_frames.clear();
    }


    RHI_RenderTarget* VulkanAPI::create_render_target(const RenderTarget* render_target)
    {
        return &(new VulkanRenderTarget())->init(render_target, render_target->render_pass->rhi_object<VulkanRenderPass>());
    }
}// namespace Engine
