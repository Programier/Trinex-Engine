#include <Graphics/render_surface.hpp>
#include <vulkan_api.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
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
        API->current_command_buffer_handle().pipelineBarrier(src_stage_mask, dest_stage_mask, {}, barrier, {}, {});
    }

    void VulkanRenderTargetState::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
    {
        m_render_pass = VulkanRenderPass::find_or_create(color_attachments, depth_stencil);

        RenderSurface* base_surface = color_attachments.empty() ? depth_stencil : color_attachments[0];

        if (base_surface == nullptr)
        {
            throw EngineException("Vulkan: Cannot initialize vulkan render target! No targets found!");
        }

        m_size = base_surface->rhi_object<VulkanSurface>()->size();
        post_init();
    }

    void VulkanRenderTargetState::post_init()
    {
        m_render_pass_info.setRenderPass(m_render_pass->m_render_pass);
        m_render_pass_info.setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(m_size.x, m_size.y)));
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
        auto m_state = state();
        vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(), m_state->m_render_pass->m_render_pass,
                                                          image_views, m_state->m_size.x, m_state->m_size.y, 1);
        m_framebuffer = API->m_device.createFramebuffer(framebuffer_create_info);

        return *this;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::destroy()
    {
        DESTROY_CALL(destroyFramebuffer, m_framebuffer);
        return *this;
    }

    VulkanRenderTargetState* VulkanRenderTargetBase::state()
    {
        return nullptr;
    }

    void VulkanRenderTargetBase::bind()
    {
        if (API->m_state.m_render_target == this)
        {
            return;
        }

        if (API->m_state.m_render_target)
        {
            API->m_state.m_render_target->unbind();
            push_barriers();
        }

        API->m_state.m_render_target = this;

        auto m_state = state();
        m_state->m_render_pass_info.setFramebuffer(m_framebuffer);

        if (API->find_current_viewport_mode() != API->m_state.m_viewport_mode)
        {
            API->viewport(API->m_state.m_viewport);
            API->scissor(API->m_state.m_scissor);
        }

        API->current_command_buffer_handle().beginRenderPass(m_state->m_render_pass_info, vk::SubpassContents::eInline);
        return;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::unbind()
    {
        if (API->m_state.m_render_target == this)
        {
            API->current_command_buffer_handle().endRenderPass();
            API->m_state.m_render_target = nullptr;
        }
        return *this;
    }

    bool VulkanRenderTargetBase::is_main_render_target()
    {
        return false;
    }

    VulkanRenderTargetBase& VulkanRenderTargetBase::size(uint32_t width, uint32_t height)
    {
        auto m_state      = state();
        m_state->m_size.x = static_cast<float>(width);
        m_state->m_size.y = static_cast<float>(height);
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

    VulkanMainRenderTargetState* VulkanWindowRenderTargetFrame::state()
    {
        return m_state;
    }

    VulkanRenderTarget::VulkanRenderTarget()
    {}

    TreeMap<VulkanRenderTarget::Key, VulkanRenderTarget*> VulkanRenderTarget::m_render_targets;

    void VulkanRenderTarget::Key::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
    {
        size_t index = 0;

        for (size_t count = color_attachments.size(); index < count; ++index)
        {
            m_color_attachments[index] = color_attachments[index]->rhi_object<VulkanSurface>();
        }

        for (; index < RHI_MAX_RT_BINDED; ++index)
        {
            m_color_attachments[index] = nullptr;
        }

        m_depth_stencil = depth_stencil ? depth_stencil->rhi_object<VulkanSurface>() : nullptr;
    }

    bool VulkanRenderTarget::Key::operator<(const Key& key) const
    {
        return std::memcmp(this, &key, sizeof(*this)) < 0;
    }

    VulkanRenderTarget* VulkanRenderTarget::find_or_create(const Span<RenderSurface*>& color_attachments,
                                                           RenderSurface* depth_stencil)
    {
        Key key;
        key.init(color_attachments, depth_stencil);
        VulkanRenderTarget*& render_target = m_render_targets[key];

        if (render_target != nullptr)
            return render_target;

        render_target = new VulkanRenderTarget();
        render_target->init(color_attachments, depth_stencil);

        return render_target;
    }

    VulkanRenderTarget& VulkanRenderTarget::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
    {
        m_state.init(color_attachments, depth_stencil);

        m_attachments.resize(color_attachments.size() + (depth_stencil ? 1 : 0));
        m_surfaces.resize(m_attachments.size());

        Index index = 0;
        for (auto& attachment : color_attachments)
        {
            const Texture2D* color_binding = attachment;
            VulkanSurface* texture         = color_binding->rhi_object<VulkanSurface>();

            trinex_check(texture, "Vulkan API: Cannot attach color texture: Texture is NULL");
            bool usage_check = texture->is_render_target_color_image();
            trinex_check(usage_check, "Vulkan API: Pixel type for color attachment must be RGBA");

            vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            m_attachments[index] = texture->create_image_view(range);
            m_surfaces[index]    = texture;
            texture->m_render_targets.insert(this);
            ++index;
        }

        if (depth_stencil)
        {
            const Texture2D* binding = depth_stencil;
            VulkanSurface* texture   = binding->rhi_object<VulkanSurface>();
            trinex_check(texture, "Vulkan API: Cannot depth attach texture: Texture is NULL");

            bool check_status = texture->is_depth_stencil_image();
            trinex_check(check_status, "Vulkan API: Pixel type for depth attachment must be Depth* or Stencil*");

            vk::ImageSubresourceRange range(texture->aspect(), 0, 1, 0, 1);
            m_attachments[index] = texture->create_image_view(range);
            m_surfaces[index]    = texture;

            texture->m_render_targets.insert(this);
        }

        post_init(m_attachments);
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

    VulkanRenderTargetState* VulkanRenderTarget::state()
    {
        return &m_state;
    }

    void VulkanRenderTarget::bind()
    {
        VulkanRenderTargetBase::bind();

        for (auto& surface : m_surfaces)
        {
            surface->layout(vk::ImageLayout::eColorAttachmentOptimal);
        }
    }

    VulkanRenderTargetBase& VulkanRenderTarget::unbind()
    {
        VulkanRenderTargetBase::unbind();
        for (auto& surface : m_surfaces)
        {
            surface->layout(vk::ImageLayout::eShaderReadOnlyOptimal);
        }
        return *this;
    }

    VulkanRenderTarget::~VulkanRenderTarget()
    {
        for (auto& image_view : m_attachments)
        {
            DESTROY_CALL(destroyImageView, image_view);
        }

        for (VulkanSurface* surface : m_surfaces)
        {
            surface->m_render_targets.erase(this);
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
        state.m_size.x      = static_cast<float>(viewport->m_swapchain->extent.width);
        state.m_size.y      = static_cast<float>(viewport->m_swapchain->extent.height);
        state.m_render_pass = VulkanRenderPass::swapchain_render_pass(vk::Format(viewport->m_swapchain->image_format));
        state.post_init();

        for (VulkanWindowRenderTargetFrame* frame : m_frames)
        {
            frame->m_state = &state;
            frame->post_init({viewport->m_image_views[index]});
            ++index;
        }
        return *this;
    }

    VulkanWindowRenderTargetFrame* VulkanWindowRenderTarget::frame()
    {
        return m_frames[m_viewport->m_buffer_index];
    }

    void VulkanWindowRenderTarget::bind()
    {
        frame()->bind();
    }

    VulkanWindowRenderTarget::~VulkanWindowRenderTarget()
    {
        for (VulkanWindowRenderTargetFrame* frame : m_frames)
        {
            delete frame;
        }
        m_frames.clear();
    }

    VulkanAPI& VulkanAPI::bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
    {
        VulkanRenderTarget* rt = VulkanRenderTarget::find_or_create(color_attachments, depth_stencil);
        rt->bind();
        return *this;
    }

    VulkanAPI& VulkanAPI::viewport(const ViewPort& viewport)
    {
        auto& m_viewport = m_state.m_viewport;
        auto new_mode    = find_current_viewport_mode();

        if (new_mode != m_state.m_viewport_mode || m_viewport != viewport)
        {
            if (new_mode != VulkanViewportMode::Undefined)
            {
                float vp_height = viewport.size.y;
                float vp_y      = viewport.pos.y;

                if (new_mode == VulkanViewportMode::Flipped)
                {
                    vp_height               = -vp_height;
                    auto render_target_size = m_state.m_current_viewport->render_target()->state()->m_size;
                    vp_y                    = render_target_size.y - vp_y;
                }

                {
                    vk::Viewport vulkan_viewport;
                    vulkan_viewport.setWidth(viewport.size.x);
                    vulkan_viewport.setHeight(vp_height);
                    vulkan_viewport.setX(viewport.pos.x);
                    vulkan_viewport.setY(vp_y);
                    vulkan_viewport.setMinDepth(viewport.min_depth);
                    vulkan_viewport.setMaxDepth(viewport.max_depth);
                    current_command_buffer_handle().setViewport(0, vulkan_viewport);
                }
            }

            m_viewport = viewport;
        }
        return *this;
    }

    ViewPort VulkanAPI::viewport()
    {
        return m_state.m_viewport;
    }

    VulkanAPI& VulkanAPI::scissor(const Scissor& scissor)
    {
        auto& m_scissor = m_state.m_scissor;
        auto new_mode   = find_current_viewport_mode();

        if (new_mode != m_state.m_viewport_mode || m_scissor != scissor)
        {
            if (new_mode != VulkanViewportMode::Undefined)
            {
                const auto& render_target_size = m_state.m_current_viewport->render_target()->state()->m_size;
                float sc_y = scissor.pos.y;

                if (new_mode == VulkanViewportMode::Flipped)
                {
                    sc_y = render_target_size.y - sc_y - scissor.size.y;
                }

                vk::Rect2D vulkan_scissor;
                vulkan_scissor.offset.setX(glm::clamp(scissor.pos.x, 0.f, render_target_size.x));
                vulkan_scissor.offset.setY(glm::clamp(sc_y, 0.f, render_target_size.y));
                vulkan_scissor.extent.setWidth(glm::clamp(scissor.size.x, 0.f, render_target_size.x - vulkan_scissor.offset.x));
                vulkan_scissor.extent.setHeight(glm::clamp(scissor.size.y, 0.f, render_target_size.y - vulkan_scissor.offset.y));
                current_command_buffer_handle().setScissor(0, vulkan_scissor);
            }

            m_scissor = scissor;
        }
        return *this;
    }

    Scissor VulkanAPI::scissor()
    {
        return m_state.m_scissor;
    }
}// namespace Engine
