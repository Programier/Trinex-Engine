#include <Graphics/framebuffer.hpp>
#include <Graphics/render_pass.hpp>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_framebuffer.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_transition_image_layout.hpp>

namespace Engine
{

#define COLOR_BUFFER_BIT 1U
#define DEPTH_STENCIL_BUFFER_BIT 2U

    VulkanFramebuffer::VulkanFramebuffer()
    {}

    VulkanFramebuffer& VulkanFramebuffer::init(const RenderTarget* render_target, VulkanRenderPass* render_pass)
    {
        destroy();
        _M_render_pass = render_pass;
        _M_size.width  = static_cast<uint32_t>(render_target->size.x);
        _M_size.height = static_cast<uint32_t>(render_target->size.y);


        _M_clear_values.resize(render_pass->attachments_count());
        _M_attachments.resize(render_pass->attachments_count());

        Index index = 0;
        for (const RenderTarget::Attachment& color_binding : render_target->color_attachments)
        {
            VulkanTexture* texture = reinterpret_cast<VulkanTexture*>(color_binding.texture);

            trinex_check(texture && "Vulkan API: Cannot attach texture: Texture is NULL");
            bool usage_check = texture->can_use_color_as_color_attachment();
            trinex_check(usage_check && "Vulkan API: Pixel type for color attachment must be RGBA");

            vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, color_binding.mip_level, 1, 0, 1);
            _M_attachments[index] = texture->get_image_view(range);

            _M_clear_values[index].color =
                    vk::ClearColorValue(Array<float, 4>({render_target->color_clear_data[index].clear_value.color.x,
                                                         render_target->color_clear_data[index].clear_value.color.y,
                                                         render_target->color_clear_data[index].clear_value.color.z,
                                                         render_target->color_clear_data[index].clear_value.color.a}));
            ++index;
        }

        if (render_pass->_M_has_depth_attachment)
        {
            auto& binding          = render_target->depth_stencil_attachment;
            VulkanTexture* texture = reinterpret_cast<VulkanTexture*>(binding.texture);
            trinex_check(texture && "Vulkan API: Cannot attach texture: Texture is NULL");

            bool check_status = texture->is_depth_stencil_image();
            trinex_check(check_status && "Vulkan API: Pixel type for depth attachment must be Depth* or Stencil*");

            vk::ImageSubresourceRange range(texture->aspect(), binding.mip_level, 1, 0, 1);
            _M_attachments[index]               = texture->get_image_view(range);
            _M_clear_values.back().depthStencil = vk::ClearDepthStencilValue(
                    render_target->depth_stencil_clear_data.clear_value.depth_stencil.depth,
                    render_target->depth_stencil_clear_data.clear_value.depth_stencil.stencil);
        }

        return create_framebuffer();
    }

    VulkanFramebuffer& VulkanFramebuffer::create_framebuffer()
    {
        DESTROY_CALL(destroyFramebuffer, _M_framebuffer);

        vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(),
                                                          _M_render_pass->_M_render_pass, _M_attachments, _M_size.width,
                                                          _M_size.height, 1);
        _M_framebuffer = API->_M_device.createFramebuffer(framebuffer_create_info);


        _M_viewport.x     = 0;
        _M_viewport.width = _M_size.width;

        if (is_custom())
        {
            _M_viewport.y      = 0;
            _M_viewport.height = _M_size.height;
        }
        else
        {

            _M_viewport.y      = _M_size.height;
            _M_viewport.height = -_M_size.height;
        }

        _M_viewport.minDepth = 0.0f;
        _M_viewport.maxDepth = 1.0f;

        _M_scissor = vk::Rect2D({0, 0}, vk::Extent2D(_M_size.width, _M_size.height));

        return init_render_pass_info();
    }

    VulkanFramebuffer& VulkanFramebuffer::init_render_pass_info()
    {
        _M_render_pass_info.setRenderPass(_M_render_pass->_M_render_pass);
        _M_render_pass_info.setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(_M_size.width, _M_size.height)));
        _M_render_pass_info.setClearValues(_M_clear_values);
        _M_is_inited = true;
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::begin_pass(size_t index)
    {
        _M_render_pass_info.setFramebuffer(_M_framebuffer);
        API->_M_command_buffer->get().beginRenderPass(_M_render_pass_info, vk::SubpassContents::eInline);
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::end_pass()
    {
        API->_M_command_buffer->get().endRenderPass();
        return *this;
    }

    bool VulkanFramebuffer::is_custom() const
    {
        return _M_render_pass != API->_M_main_render_pass;
    }

    VulkanFramebuffer& VulkanFramebuffer::destroy()
    {
        API->wait_idle();

        DESTROY_CALL(destroyFramebuffer, _M_framebuffer);

        if (is_custom())
        {
            for (auto& image_view : _M_attachments)
            {
                DESTROY_CALL(destroyImageView, image_view);
            }
        }

        return *this;
    }

    void VulkanFramebuffer::viewport(const ViewPort& viewport)
    {
        _M_viewport.x        = viewport.pos.x;
        _M_viewport.width    = viewport.size.x;
        _M_viewport.minDepth = viewport.min_depth;
        _M_viewport.maxDepth = viewport.max_depth;

        if (is_custom())
        {
            _M_viewport.y      = viewport.pos.y;
            _M_viewport.height = viewport.size.y;
        }
        else
        {
            // Revert Y
            _M_viewport.height = -viewport.size.y;
            _M_viewport.y      = _M_size.height - viewport.pos.y;
        }

        if (this == API->_M_state->_M_framebuffer)
            set_viewport();
    }

    VulkanFramebuffer& VulkanFramebuffer::set_viewport()
    {
        API->_M_command_buffer->get().setViewport(0, _M_viewport);
        return *this;
    }

    void VulkanFramebuffer::scissor(const Scissor& scissor)
    {
        _M_scissor.offset.x      = scissor.pos.x;
        _M_scissor.extent.width  = scissor.size.x;
        _M_scissor.extent.height = scissor.size.y;

        if (is_custom())
        {
            _M_scissor.offset.y = scissor.pos.y;
        }
        else
        {
            _M_scissor.offset.y = _M_size.height - scissor.pos.y - scissor.size.y;
        }

        if (this == API->_M_state->_M_framebuffer)
            set_scissor();
    }

    VulkanFramebuffer& VulkanFramebuffer::set_scissor()
    {
        API->_M_command_buffer->get().setScissor(0, _M_scissor);
        return *this;
    }

    void VulkanFramebuffer::bind(uint_t index)
    {
        if (API->_M_state->_M_framebuffer)
        {
            API->_M_state->_M_framebuffer->unbind();
        }

        API->_M_state->_M_framebuffer = this;

        begin_pass(index).set_viewport().set_scissor();
    }

    VulkanFramebuffer& VulkanFramebuffer::unbind()
    {
        if (API->_M_state->_M_framebuffer == this)
        {
            API->_M_state->_M_framebuffer = nullptr;
            return end_pass();
        }

        return *this;
    }

    void VulkanFramebuffer::clear_color(const ColorClearValue& color, byte layout)
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

    void VulkanFramebuffer::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        if (_M_render_pass->_M_has_depth_attachment)
        {
            _M_clear_values.back().setDepthStencil(vk::ClearDepthStencilValue(value.depth, value.stencil));
        }
    }

    VulkanFramebuffer& VulkanFramebuffer::size(uint32_t width, uint32_t height)
    {
        _M_size.width  = width;
        _M_size.height = height;
        return *this;
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        destroy();
    }

    VulkanMainFrameBuffer::VulkanMainFrameBuffer()
    {}


    VulkanMainFrameBuffer& VulkanMainFrameBuffer::destroy()
    {
        for (VulkanFramebuffer* framebuffer : _M_framebuffers)
        {
            framebuffer->destroy();
        }
        return *this;
    }

    void VulkanMainFrameBuffer::resize_count(size_t new_count)
    {
        _M_framebuffers.resize(new_count);
        for (VulkanFramebuffer*& framebuffer : _M_framebuffers)
        {
            if (framebuffer == nullptr)
                framebuffer = new VulkanFramebuffer();
        }
    }

    static void init_main_framebuffer(VulkanFramebuffer* buffer, uint_t index)
    {
        buffer->_M_size.height = API->_M_swap_chain->_M_extent.height;
        buffer->_M_size.width  = API->_M_swap_chain->_M_extent.width;

        buffer->_M_attachments.resize(1);
        buffer->_M_attachments[0] = API->_M_swap_chain->_M_image_views[index];
        buffer->_M_render_pass    = API->_M_main_render_pass;
        buffer->create_framebuffer();
    }

    VulkanMainFrameBuffer& VulkanMainFrameBuffer::init()
    {
        uint_t index = 0;
        for (VulkanFramebuffer* framebuffer : _M_framebuffers)
        {
            init_main_framebuffer(framebuffer, index++);
        }

        return *this;
    }

    Vector<VulkanFramebuffer*> _M_framebuffers;

    VulkanMainFrameBuffer& init();

    void VulkanMainFrameBuffer::bind(uint_t)
    {
        _M_framebuffers[API->swapchain_image_index().value]->bind(0);
    }

    void VulkanMainFrameBuffer::viewport(const ViewPort& viewport)
    {
        for (VulkanFramebuffer* framebuffer : _M_framebuffers)
        {
            framebuffer->viewport(viewport);
        }
    }

    void VulkanMainFrameBuffer::scissor(const Scissor& scissor)
    {
        for (VulkanFramebuffer* framebuffer : _M_framebuffers)
        {
            framebuffer->scissor(scissor);
        }
    }

    void VulkanMainFrameBuffer::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        for (VulkanFramebuffer* framebuffer : _M_framebuffers)
        {
            framebuffer->clear_depth_stencil(value);
        }
    }

    void VulkanMainFrameBuffer::clear_color(const ColorClearValue& color, byte layout)
    {
        for (VulkanFramebuffer* framebuffer : _M_framebuffers)
        {
            framebuffer->clear_color(color, layout);
        }
    }

    VulkanMainFrameBuffer::~VulkanMainFrameBuffer()
    {
        for (VulkanFramebuffer* framebuffer : _M_framebuffers)
        {
            delete framebuffer;
        }

        _M_framebuffers.clear();
    }

    VulkanMainFrameBuffer& VulkanMainFrameBuffer::size(uint32_t width, uint32_t height)
    {
        for (VulkanFramebuffer* framebuffer : _M_framebuffers)
        {
            framebuffer->size(width, height);
        }
        return *this;
    }


    RHI_RenderTarget* VulkanAPI::window_render_target()
    {
        return API->_M_main_framebuffer;
    }

    RHI_RenderTarget* VulkanAPI::create_render_target(const RenderTarget* render_target)
    {
        return &(new VulkanFramebuffer())
                        ->init(render_target, render_target->render_pass->rhi_object<VulkanRenderPass>());
    }
}// namespace Engine
