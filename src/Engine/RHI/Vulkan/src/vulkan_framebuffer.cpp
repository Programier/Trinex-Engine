#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <vulkan_api.hpp>
#include <vulkan_framebuffer.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_transition_image_layout.hpp>

namespace Engine
{

    void VulkanRenderTargetFrame::init(struct VulkanRenderTarget* owner, const RenderTarget* render_target,
                                       struct VulkanRenderPass* render_pass, Index frame)
    {
        destroy();
        _M_owner = owner;

        _M_attachments.resize(render_pass->attachments_count());

        Index index = 0;
        for (const Texture2D* color_binding : render_target->frame(frame)->color_attachments)
        {
            VulkanTexture* texture = color_binding->rhi_object<VulkanTexture>();

            trinex_check(texture && "Vulkan API: Cannot attach color texture: Texture is NULL");
            bool usage_check = texture->can_use_color_as_color_attachment();
            trinex_check(usage_check && "Vulkan API: Pixel type for color attachment must be RGBA");

            vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            _M_attachments[index] = texture->get_image_view(range);
            ++index;
        }

        if (render_pass->_M_has_depth_attachment)
        {
            const Texture2D* binding = render_target->frame(frame)->depth_stencil_attachment;
            VulkanTexture* texture   = binding->rhi_object<VulkanTexture>();
            trinex_check(texture && "Vulkan API: Cannot depth attach texture: Texture is NULL");

            bool check_status = texture->is_depth_stencil_image();
            trinex_check(check_status && "Vulkan API: Pixel type for depth attachment must be Depth* or Stencil*");

            vk::ImageSubresourceRange range(texture->aspect(), 0, 1, 0, 1);
            _M_attachments[index] = texture->get_image_view(range);
        }
    }

    void VulkanRenderTargetFrame::post_init()
    {
        // Initialize framebuffer
        vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(),
                                                          _M_owner->_M_render_pass->_M_render_pass, _M_attachments,
                                                          _M_owner->_M_size.width, _M_owner->_M_size.height, 1);
        _M_framebuffer = API->_M_device.createFramebuffer(framebuffer_create_info);
    }


    void VulkanRenderTargetFrame::push_barriers(size_t count)
    {
        auto src_stage_mask =
                vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eColorAttachmentOutput;
        auto dest_stage_mask = vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader |
                               vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader |
                               vk::PipelineStageFlagBits::eTransfer;


        std::vector<vk::MemoryBarrier> barriers(count);

        auto src_access_mask =
                vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
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

    void VulkanRenderTargetFrame::bind()
    {
        if (API->_M_state->_M_framebuffer == this)
            return;


        if (API->_M_state->_M_framebuffer)
        {
            size_t count = API->_M_state->_M_framebuffer->_M_attachments.size();
            API->_M_state->_M_framebuffer->unbind();
            push_barriers(count);
        }


        API->_M_state->_M_framebuffer = this;
        _M_owner->_M_render_pass_info.setFramebuffer(_M_framebuffer);
        API->current_command_buffer().beginRenderPass(_M_owner->_M_render_pass_info, vk::SubpassContents::eInline);

        update_viewport().update_scissors();
    }

    void VulkanRenderTargetFrame::unbind()
    {
        if (API->_M_state->_M_framebuffer == this)
        {
            API->current_command_buffer().endRenderPass();
            API->_M_state->_M_framebuffer = nullptr;
        }
    }

    VulkanRenderTargetFrame& VulkanRenderTargetFrame::update_viewport()
    {
        API->current_command_buffer().setViewport(0, _M_owner->_M_viewport);
        return *this;
    }

    VulkanRenderTargetFrame& VulkanRenderTargetFrame::update_scissors()
    {
        API->current_command_buffer().setScissor(0, _M_owner->_M_scissor);
        return *this;
    }

    void VulkanRenderTargetFrame::destroy()
    {
        DESTROY_CALL(destroyFramebuffer, _M_framebuffer);

        if (!is_main_frame())
        {
            for (auto& image_view : _M_attachments)
            {
                DESTROY_CALL(destroyImageView, image_view);
            }
        }
    }

    bool VulkanRenderTargetFrame::is_main_frame()
    {
        return false;
    }

    vk::Semaphore* VulkanRenderTargetFrame::image_present_semaphore()
    {
        return nullptr;
    }

    VulkanMainRenderTargetFrame::VulkanMainRenderTargetFrame()
    {
        _M_image_present  = API->_M_device.createSemaphore(vk::SemaphoreCreateInfo());
        _M_command_buffer = API->_M_device.allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(API->_M_command_pool, vk::CommandBufferLevel::ePrimary, 1))[0];

        _M_render_finished = API->_M_device.createSemaphore(vk::SemaphoreCreateInfo());
        _M_fence           = API->_M_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }

    VulkanMainRenderTargetFrame::~VulkanMainRenderTargetFrame()
    {
        DESTROY_CALL(destroySemaphore, _M_image_present);
        DESTROY_CALL(destroyFence, _M_fence);
        DESTROY_CALL(destroySemaphore, _M_render_finished);
    }

    void VulkanMainRenderTargetFrame::wait()
    {
        while (vk::Result::eTimeout == API->_M_device.waitForFences(_M_fence, VK_TRUE, UINT64_MAX))
        {
        }

        API->_M_device.resetFences(_M_fence);
    }

    bool VulkanMainRenderTargetFrame::is_main_frame()
    {
        return true;
    }

    vk::Semaphore* VulkanMainRenderTargetFrame::image_present_semaphore()
    {
        return &_M_image_present;
    }

    VulkanRenderTargetFrame::~VulkanRenderTargetFrame()
    {}

    VulkanRenderTarget& VulkanRenderTarget::init(const RenderTarget* render_target, VulkanRenderPass* render_pass)
    {
        _M_render_pass = render_pass;
        _M_size.width  = static_cast<uint32_t>(render_target->size.x);
        _M_size.height = static_cast<uint32_t>(render_target->size.y);

        _M_clear_values.resize(render_pass->attachments_count());

        for (Index index = 0, count = render_pass->attachments_count(); index < count; index++)
        {
            _M_clear_values[index].color = vk::ClearColorValue(
                    Array<float, 4>({render_target->color_clear[index].x, render_target->color_clear[index].y,
                                     render_target->color_clear[index].z, render_target->color_clear[index].a}));
        }

        if (render_pass->_M_has_depth_attachment)
        {
            _M_clear_values.back().depthStencil = vk::ClearDepthStencilValue(
                    render_target->depth_stencil_clear.depth, render_target->depth_stencil_clear.stencil);
        }


        // Initialize frames

        for (uint32_t i = 0; i < API->render_target_buffer_count(); i++)
        {
            VulkanRenderTargetFrame* frame = new VulkanRenderTargetFrame();
            frame->init(this, render_target, render_pass, i);
            _M_frames.push_back(frame);
        }

        return post_init();
    }

    VulkanRenderTarget& VulkanRenderTarget::post_init()
    {
        _M_viewport.x     = 0;
        _M_viewport.width = static_cast<float>(_M_size.width);

        if (is_main_render_target())
        {
            _M_viewport.y      = static_cast<float>(_M_size.height);
            _M_viewport.height = -static_cast<float>(_M_size.height);
        }
        else
        {
            _M_viewport.y      = 0;
            _M_viewport.height = static_cast<float>(_M_size.height);
        }

        _M_viewport.minDepth = 0.0f;
        _M_viewport.maxDepth = 1.0f;

        _M_scissor = vk::Rect2D({0, 0}, vk::Extent2D(_M_size.width, _M_size.height));

        // Initialize render_pass info
        _M_render_pass_info.setRenderPass(_M_render_pass->_M_render_pass);
        _M_render_pass_info.setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(_M_size.width, _M_size.height)));
        _M_render_pass_info.setClearValues(_M_clear_values);

        for (VulkanRenderTargetFrame* frame : _M_frames)
        {
            frame->post_init();
        }

        return *this;
    }

    VulkanRenderTarget& VulkanRenderTarget::destroy()
    {
        API->wait_idle();

        for (VulkanRenderTargetFrame* frame : _M_frames)
        {
            frame->destroy();
            delete frame;
        }

        return *this;
    }

    void VulkanRenderTarget::viewport(const ViewPort& viewport)
    {
        _M_viewport.x        = viewport.pos.x;
        _M_viewport.width    = viewport.size.x;
        _M_viewport.minDepth = viewport.min_depth;
        _M_viewport.maxDepth = viewport.max_depth;

        if (is_main_render_target())
        {
            // Revert Y
            _M_viewport.height = -viewport.size.y;
            _M_viewport.y      = _M_size.height - viewport.pos.y;
        }
        else
        {
            _M_viewport.y      = viewport.pos.y;
            _M_viewport.height = viewport.size.y;
        }


        if (API->_M_state->_M_framebuffer == _M_frames[API->_M_current_buffer])
            API->_M_state->_M_framebuffer->update_viewport();
    }

    void VulkanRenderTarget::scissor(const Scissor& scissor)
    {
        _M_scissor.offset.x      = scissor.pos.x;
        _M_scissor.extent.width  = scissor.size.x;
        _M_scissor.extent.height = scissor.size.y;

        _M_scissor.offset.y = scissor.pos.y;

        if (API->_M_state->_M_framebuffer == _M_frames[API->_M_current_buffer])
            API->_M_state->_M_framebuffer->update_scissors();
    }


    Index VulkanRenderTarget::bind()
    {
        _M_frames[API->_M_current_buffer]->bind();
        return API->_M_current_buffer;
    }

    void VulkanRenderTarget::clear_color(const ColorClearValue& color, byte layout)
    {
        byte layouts_count =
                static_cast<byte>(_M_clear_values.size()) - static_cast<byte>(_M_render_pass->_M_has_depth_attachment);

        if (layout < layouts_count)
        {
            _M_clear_values[layout].setColor(
                    vk::ClearColorValue(Array<float, 4>({color.x, color.y, color.z, color.a})));
        }
        else
        {
            vulkan_debug_log("Vulkan API", "Incorrect layout index!");
        }
    }

    void VulkanRenderTarget::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        if (_M_render_pass->_M_has_depth_attachment)
        {
            _M_clear_values.back().setDepthStencil(vk::ClearDepthStencilValue(value.depth, value.stencil));
        }
    }

    bool VulkanRenderTarget::is_main_render_target()
    {
        return false;
    }

    VulkanRenderTarget& VulkanRenderTarget::size(uint32_t width, uint32_t height)
    {
        _M_size.width  = width;
        _M_size.height = height;
        return *this;
    }

    VulkanRenderTarget::~VulkanRenderTarget()
    {
        destroy();
    }


    VulkanMainFrameBuffer& VulkanMainFrameBuffer::destroy()
    {
        for (VulkanRenderTargetFrame* frame : _M_frames)
        {
            frame->destroy();
        }
        return *this;
    }

    void VulkanMainFrameBuffer::resize_count(size_t new_count)
    {
        _M_frames.resize(new_count);
        for (VulkanRenderTargetFrame*& frame : _M_frames)
        {
            if (frame == nullptr)
                frame = new VulkanMainRenderTargetFrame();
        }
    }

    static void init_main_framebuffer(VulkanRenderTargetFrame* frame, uint_t index)
    {
        frame->_M_attachments.resize(1);
        frame->_M_attachments[0] = API->_M_swap_chain->_M_image_views[index];
    }

    VulkanMainFrameBuffer& VulkanMainFrameBuffer::init()
    {
        uint_t index = 0;

        _M_size.height = API->_M_swap_chain->_M_extent.height;
        _M_size.width  = API->_M_swap_chain->_M_extent.width;
        _M_render_pass = API->_M_main_render_pass;

        for (VulkanRenderTargetFrame* frame : _M_frames)
        {
            frame->_M_owner = this;
            init_main_framebuffer(frame, index++);
        }

        post_init();

        return *this;
    }

    bool VulkanMainFrameBuffer::is_destroyable() const
    {
        return false;
    }

    bool VulkanMainFrameBuffer::is_main_render_target()
    {
        return true;
    }

    RHI_RenderTarget* VulkanAPI::window_render_target()
    {
        return API->_M_main_framebuffer;
    }

    RHI_RenderTarget* VulkanAPI::create_render_target(const RenderTarget* render_target)
    {
        if (render_target->frames_count() != render_target_buffer_count())
            throw EngineException("Frames count is mismatch with API requirements");
        return &(new VulkanRenderTarget())
                        ->init(render_target, render_target->render_pass->rhi_object<VulkanRenderPass>());
    }

    vk::CommandBuffer& VulkanAPI::current_command_buffer()
    {
        return reinterpret_cast<VulkanMainRenderTargetFrame*>(API->_M_main_framebuffer->_M_frames[_M_image_index])
                ->_M_command_buffer;
    }

    struct VulkanMainRenderTargetFrame* VulkanAPI::current_main_frame()
    {
        return reinterpret_cast<VulkanMainRenderTargetFrame*>(API->_M_main_framebuffer->_M_frames[_M_image_index]);
    }
}// namespace Engine
