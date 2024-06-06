#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/render_target_texture.hpp>
#include <vulkan_api.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{
    static FORCE_INLINE void push_barriers()
    {
        auto src_stage_mask  = vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eColorAttachmentOutput;
        auto dest_stage_mask = vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader |
                               vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader |
                               vk::PipelineStageFlagBits::eTransfer;


        vk::MemoryBarrier barrier;

        auto src_access_mask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        auto dst_access_mask = vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead |
                               vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite |
                               vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;


        barrier.setSrcAccessMask(src_access_mask);
        barrier.setDstAccessMask(dst_access_mask);
        API->current_command_buffer().pipelineBarrier(src_stage_mask, dest_stage_mask, {}, barrier, {}, {});
    }

    void VulkanRenderTargetState::init(const RenderTarget* render_target, VulkanRenderPass* render_pass)
    {
        m_render_pass = render_pass;
        m_size.width  = static_cast<uint32_t>(render_target->size.x);
        m_size.height = static_cast<uint32_t>(render_target->size.y);
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
    }

    bool VulkanRenderTargetState::is_main_render_target_state()
    {
        return false;
    }

    bool VulkanMainRenderTargetState::is_main_render_target_state()
    {
        return true;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::post_init(const Vector<vk::ImageView>& image_views)
    {
        vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(), m_state->m_render_pass->m_render_pass,
                                                          image_views, m_state->m_size.width, m_state->m_size.height, 1);
        m_framebuffer = API->m_device.createFramebuffer(framebuffer_create_info);

        return *this;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::destroy()
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

        if (API->m_state->m_render_target == this)
            API->m_state->m_render_target->update_viewport();
    }

    void VulkanRenderTargetBase::scissor(const Scissor& scissor)
    {
        m_state->m_scissor.offset.x      = scissor.pos.x;
        m_state->m_scissor.extent.width  = scissor.size.x;
        m_state->m_scissor.extent.height = scissor.size.y;

        m_state->m_scissor.offset.y = scissor.pos.y;

        if (API->m_state->m_render_target == this)
            API->m_state->m_render_target->update_scissors();
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


    void VulkanRenderTargetBase::bind()
    {
        if (API->m_state->m_render_target == this)
        {
            return;
        }

        if (API->m_state->m_render_target)
        {
            API->m_state->m_render_target->unbind();
            push_barriers();
        }

        API->m_state->m_render_target = this;

        m_state->m_render_pass_info.setFramebuffer(m_framebuffer);

        API->current_command_buffer().beginRenderPass(m_state->m_render_pass_info, vk::SubpassContents::eInline);
        update_viewport().update_scissors();
        return;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::unbind()
    {
        if (API->m_state->m_render_target == this)
        {
            API->current_command_buffer().endRenderPass();
            API->m_state->m_render_target = nullptr;
        }
        return *this;
    }

    void VulkanRenderTargetBase::clear_color(const ColorClearValue& color, byte layout)
    {}

    void VulkanRenderTargetBase::clear_depth_stencil(const DepthStencilClearValue& value)
    {}

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
        DESTROY_CALL(destroyFramebuffer, m_framebuffer);
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
        m_color_textures.resize(render_pass->color_attachments_count());
        state.init(target, render_pass);


        m_attachments.resize(render_pass->attachments_count());

        Index index = 0;
        for (auto& attachment : target->color_attachments)
        {
            const Texture2D* color_binding = attachment.ptr();
            VulkanTexture* texture         = color_binding->rhi_object<VulkanTexture>();

            trinex_check(texture, "Vulkan API: Cannot attach color texture: Texture is NULL");
            bool usage_check = texture->is_render_target_color_image();
            trinex_check(usage_check, "Vulkan API: Pixel type for color attachment must be RGBA");

            vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            m_attachments[index] = texture->create_image_view(range);
            on_color_attachment(texture, index);
            ++index;
        }

        if (render_pass->m_has_depth_attachment)
        {
            const Texture2D* binding = target->depth_stencil_attachment.ptr();
            VulkanTexture* texture   = binding->rhi_object<VulkanTexture>();
            trinex_check(texture, "Vulkan API: Cannot depth attach texture: Texture is NULL");

            bool check_status = texture->is_depth_stencil_image();
            trinex_check(check_status, "Vulkan API: Pixel type for depth attachment must be Depth* or Stencil*");

            vk::ImageSubresourceRange range(texture->aspect(), 0, 1, 0, 1);
            m_attachments[index] = texture->create_image_view(range);
            on_depth_stencil_attachment(texture, index);
        }

        post_init(m_attachments);
        return *this;
    }


    VulkanRenderTarget& VulkanRenderTarget::on_color_attachment(struct VulkanTexture* texture, Index index)
    {
        if (index >= m_color_textures.size())
        {
            m_color_textures.resize(index + 1);
        }

        m_color_textures[index] = texture;
        return *this;
    }

    VulkanRenderTarget& VulkanRenderTarget::on_depth_stencil_attachment(struct VulkanTexture* texture, Index index)
    {
        m_depth_texture = texture;
        return *this;
    }

    VulkanRenderTarget& VulkanRenderTarget::destroy()
    {
        VulkanRenderTargetBase::destroy();

        for (auto& image_view : m_attachments)
        {
            DESTROY_CALL(destroyImageView, image_view);
        }

        return *this;
    }

    void VulkanRenderTarget::clear_color(const ColorClearValue& color, byte layout)
    {
        if (m_color_textures.empty() || m_color_textures.size() <= static_cast<size_t>(layout))
            return;

        bool is_binded = API->m_state->m_render_target == this;

        if (is_binded)
        {
            unbind();
        }

        VulkanTexture* texture = m_color_textures[layout];
        auto current_layout    = texture->layout();
        auto& cmd              = API->current_command_buffer();

        texture->change_layout(vk::ImageLayout::eTransferDstOptimal, cmd);
        cmd.clearColorImage(texture->image(), vk::ImageLayout::eTransferDstOptimal,
                            vk::ClearColorValue(color.r, color.g, color.b, color.a),
                            vk::ImageSubresourceRange(texture->aspect(), texture->base_mipmap(), 1, 0, 1));
        texture->change_layout(current_layout, cmd);

        if (is_binded)
        {
            bind();
        }
    }

    void VulkanRenderTarget::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        if (m_depth_texture == nullptr)
            return;

        bool is_binded = API->m_state->m_render_target == this;

        if (is_binded)
        {
            unbind();
        }

        auto current_layout = m_depth_texture->layout();
        auto& cmd           = API->current_command_buffer();


        m_depth_texture->change_layout(vk::ImageLayout::eTransferDstOptimal, cmd);
        cmd.clearDepthStencilImage(m_depth_texture->image(), vk::ImageLayout::eTransferDstOptimal,
                                   vk::ClearDepthStencilValue(value.depth, value.stencil),
                                   vk::ImageSubresourceRange(m_depth_texture->aspect(), m_depth_texture->base_mipmap(), 1, 0, 1));
        m_depth_texture->change_layout(current_layout, cmd);

        if (is_binded)
        {
            bind();
        }
    }


    VulkanRenderTarget::~VulkanRenderTarget()
    {
        for (auto& image_view : m_attachments)
        {
            DESTROY_CALL(destroyImageView, image_view);
        }
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
            frame->post_init({viewport->m_image_views[index]});
            ++index;
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

    void VulkanWindowRenderTarget::bind()
    {
        frame()->bind();
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

    static FORCE_INLINE void transition_swapchain_image_to_transfer_src(vk::Image image, vk::CommandBuffer& cmd)
    {
        static vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        vk::ImageMemoryBarrier barrier(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite,
                                       vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferDstOptimal,
                                       VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, range);
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
    }

    static FORCE_INLINE void transition_swapchain_image_to_present_src(vk::Image image, vk::CommandBuffer& cmd)
    {
        static vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        vk::ImageMemoryBarrier barrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eNone,
                                       vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
                                       VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, range);
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe, {}, {}, {}, barrier);
    }

    void VulkanWindowRenderTarget::clear_color(const ColorClearValue& color, byte layout)
    {
        vk::Image img = m_viewport->m_images[m_viewport->m_buffer_index];

        auto& cmd                                    = API->current_command_buffer();
        VulkanWindowRenderTargetFrame* current_frame = frame();
        bool is_binded                               = API->m_state->m_render_target == current_frame;

        if (is_binded)
        {
            current_frame->unbind();
        }

        transition_swapchain_image_to_transfer_src(img, cmd);
        cmd.clearColorImage(img, vk::ImageLayout::eTransferDstOptimal, vk::ClearColorValue(color.r, color.g, color.b, color.a),
                            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        transition_swapchain_image_to_present_src(img, cmd);

        if (is_binded)
        {
            current_frame->bind();
        }
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
