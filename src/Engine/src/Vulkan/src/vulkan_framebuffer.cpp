#include <Core/predef.hpp>
#include <vulkan_api.hpp>
#include <vulkan_async_command_buffer.hpp>
#include <vulkan_framebuffer.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_transition_image_layout.hpp>

namespace Engine
{

#define COLOR_BUFFER_BIT 1U
#define DEPTH_STENCIL_BUFFER_BIT 2U

    VulkanFramebuffer::VulkanFramebuffer()
    {
        _M_instance_address = this;
    }

    VulkanFramebuffer& VulkanFramebuffer::init(const FrameBufferCreateInfo& info)
    {
        _M_is_custom = 1;
        destroy();

        _M_buffers.resize(info.buffers.size());

        Index buffer_index = 0;
        for (auto& buffer : info.buffers)
        {
            auto& vulkan_buffer = _M_buffers[buffer_index++];

            if (buffer_index == 1)
            {
                if (buffer.depth_stencil_attachment.has_value())
                {
                    _M_depth_attachment_renference = new vk::AttachmentReference();
                }

                _M_size.width  = static_cast<uint32_t>(info.size.x);
                _M_size.height = static_cast<uint32_t>(info.size.y);

                size_t depth_images_count                = _M_depth_attachment_renference ? 1 : 0;
                size_t attachment_descriptions_base_size = buffer.color_attachments.size();

                _M_attachment_descriptions.resize(attachment_descriptions_base_size + depth_images_count);
                _M_color_attachment_references.resize(attachment_descriptions_base_size);
                _M_clear_values.resize(attachment_descriptions_base_size + depth_images_count);
            }

            vulkan_buffer._M_attachments.resize(_M_attachment_descriptions.size());

            Index index = 0;
            for (const FrameBufferAttachment& color_binding : buffer.color_attachments)
            {
                VulkanTexture* texture = GET_TYPE(VulkanTexture, color_binding.texture_id);
                trinex_check(texture && "Vulkan API: Cannot attach texture: Texture is NULL");
                bool usage_check = texture->can_use_color_as_color_attachment();
                trinex_check(usage_check && "Vulkan API: Pixel type for color attachment must be RGBA");
                auto& t_state = texture->state;

                if (buffer_index == 1)
                {
                    vk::AttachmentLoadOp clear_op = info.color_clear_data[index].clear_on_bind
                                                            ? vk::AttachmentLoadOp::eClear
                                                            : vk::AttachmentLoadOp::eLoad;

                    _M_attachment_descriptions[index] = vk::AttachmentDescription(
                            vk::AttachmentDescriptionFlags(), t_state.format, vk::SampleCountFlagBits::e1, clear_op,
                            vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
                            vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eShaderReadOnlyOptimal);

                    _M_color_attachment_references[index] =
                            vk::AttachmentReference(index, vk::ImageLayout::eColorAttachmentOptimal);
                }

                vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, color_binding.mip_level, 1, 0, 1);
                vulkan_buffer._M_attachments[index] =
                        GET_TYPE(VulkanTexture, color_binding.texture_id)->get_image_view(range);

                _M_clear_values[index].color =
                        vk::ClearColorValue(Array<float, 4>({info.color_clear_data[index].clear_value.color.x,
                                                             info.color_clear_data[index].clear_value.color.y,
                                                             info.color_clear_data[index].clear_value.color.z,
                                                             info.color_clear_data[index].clear_value.color.a}));
                ++index;
            }

            if (_M_depth_attachment_renference)
            {
                auto& binding          = buffer.depth_stencil_attachment.value();
                VulkanTexture* texture = GET_TYPE(VulkanTexture, binding.texture_id);
                trinex_check(texture && "Vulkan API: Cannot attach texture: Texture is NULL");

                bool check_status = texture->is_depth_stencil_image();
                trinex_check(check_status && "Vulkan API: Pixel type for depth attachment must be Depth* or Stencil*");
                auto& t_state = texture->state;

                if (buffer_index == 1)
                {
                    vk::AttachmentLoadOp clear_op = info.depth_stencil_clear_data.clear_on_bind
                                                            ? vk::AttachmentLoadOp::eClear
                                                            : vk::AttachmentLoadOp::eLoad;

                    _M_attachment_descriptions[index] = vk::AttachmentDescription(
                            {}, t_state.format, vk::SampleCountFlagBits::e1, clear_op, vk::AttachmentStoreOp::eStore,
                            vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                            vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);


                    (*_M_depth_attachment_renference) =
                            vk::AttachmentReference(index, vk::ImageLayout::eDepthStencilAttachmentOptimal);
                }

                vk::ImageSubresourceRange range(texture->_M_image_aspect, binding.mip_level, 1, 0, 1);
                vulkan_buffer._M_attachments[index] = texture->get_image_view(range);
                _M_clear_values.back().depthStencil =
                        vk::ClearDepthStencilValue(info.depth_stencil_clear_data.clear_value.depth_stencil.depth,
                                                   info.depth_stencil_clear_data.clear_value.depth_stencil.stencil);
            }
        }
        return create_render_pass().create_framebuffer();
    }

    VulkanFramebuffer& VulkanFramebuffer::create_render_pass()
    {
        if (_M_is_custom)
            DESTROY_CALL(destroyRenderPass, _M_render_pass);

        _M_subpass = vk::SubpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
                                            _M_color_attachment_references, {}, _M_depth_attachment_renference);


        vk::PipelineStageFlags pipeline_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::AccessFlags access_flags          = vk::AccessFlagBits::eColorAttachmentWrite;
        if (_M_depth_attachment_renference)
        {
            pipeline_flags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
            access_flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        }

        _M_dependency  = vk::SubpassDependency((_M_is_custom ? 0 : VK_SUBPASS_EXTERNAL), 0, pipeline_flags,
                                               pipeline_flags, {}, access_flags, vk::DependencyFlagBits::eByRegion);
        _M_render_pass = API->_M_device.createRenderPass(vk::RenderPassCreateInfo(
                vk::RenderPassCreateFlags(), _M_attachment_descriptions, _M_subpass, _M_dependency));
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::create_framebuffer()
    {
        if (!_M_render_pass)
            create_render_pass();

        for (auto& vulkan_buffer : _M_buffers)
        {
            DESTROY_CALL(destroyFramebuffer, vulkan_buffer._M_framebuffer);

            vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(), _M_render_pass,
                                                              vulkan_buffer._M_attachments, _M_size.width,
                                                              _M_size.height, 1);
            vulkan_buffer._M_framebuffer = API->_M_device.createFramebuffer(framebuffer_create_info);
        }

        if (!_M_is_inited)
        {
            _M_viewport.x     = 0;
            _M_viewport.width = _M_size.width;

            if (_M_is_custom)
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

            _M_scissor = {{0, 0}, vk::Extent2D(_M_size.width, _M_size.height)};
        }
        else
        {}

        return init_render_pass_info();
    }

    VulkanFramebuffer& VulkanFramebuffer::init_render_pass_info()
    {
        _M_render_pass_info.setRenderPass(_M_render_pass);
        _M_render_pass_info.setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(_M_size.width, _M_size.height)));
        _M_render_pass_info.setClearValues(_M_clear_values);
        _M_is_inited = true;
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::begin_pass(struct ThreadedCommandBuffer* command_buffer, size_t index)
    {
        _M_render_pass_info.setFramebuffer(_M_buffers[index]._M_framebuffer);
        command_buffer->_M_buffer.beginRenderPass(_M_render_pass_info, vk::SubpassContents::eInline);
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::end_pass(ThreadedCommandBuffer* command_buffer)
    {
        command_buffer->_M_buffer.endRenderPass();
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::destroy()
    {
        API->wait_idle();
        for (auto& vulkan_buffer : _M_buffers)
        {
            DESTROY_CALL(destroyFramebuffer, vulkan_buffer._M_framebuffer);

            if (_M_is_custom)
            {
                for (auto& image_view : vulkan_buffer._M_attachments)
                {
                    DESTROY_CALL(destroyImageView, image_view);
                }
            }
        }

        if (_M_is_custom)
            DESTROY_CALL(destroyRenderPass, _M_render_pass);


        if (_M_depth_attachment_renference)
            delete _M_depth_attachment_renference;
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::update_viewport(const ViewPort& viewport)
    {
        _M_viewport.x        = viewport.pos.x;
        _M_viewport.width    = viewport.size.x;
        _M_viewport.minDepth = viewport.min_depth;
        _M_viewport.maxDepth = viewport.max_depth;

        if (_M_is_custom)
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

        if (API->_M_current_command_buffer &&
            this == API->_M_current_command_buffer->get_threaded_command_buffer()->_M_current_framebuffer)
            set_viewport();

        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::set_viewport()
    {
        API->_M_current_command_buffer->get()->setViewport(0, _M_viewport);
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::update_scissor(const Scissor& scissor)
    {
        _M_scissor.offset.x      = scissor.pos.x;
        _M_scissor.extent.width  = scissor.size.x;
        _M_scissor.extent.height = scissor.size.y;

        if (_M_is_custom)
        {
            _M_scissor.offset.y = scissor.pos.y;
        }
        else
        {
            _M_scissor.offset.y = _M_size.height - scissor.pos.y - scissor.size.y;
        }

        if (API->_M_current_command_buffer &&
            this == API->_M_current_command_buffer->get_threaded_command_buffer()->_M_current_framebuffer)
            set_scissor();

        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::set_scissor()
    {
        API->_M_current_command_buffer->get()->setScissor(0, _M_scissor);
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::bind(size_t index)
    {
        auto cmd = API->_M_current_command_buffer->get_threaded_command_buffer();

        if (cmd->_M_current_framebuffer)
        {
            cmd->_M_current_framebuffer->unbind(cmd);
        }

        cmd->_M_current_framebuffer = this;

        return begin_pass(cmd, index).set_viewport().set_scissor();
    }

    VulkanFramebuffer& VulkanFramebuffer::unbind(struct ThreadedCommandBuffer* command_buffer)
    {
        command_buffer->_M_current_framebuffer = nullptr;
        return end_pass(command_buffer);
    }

    VulkanFramebuffer& VulkanFramebuffer::clear_color(const ColorClearValue& color, byte layout)
    {
        byte layouts_count = static_cast<byte>(_M_clear_values.size()) -
                             static_cast<byte>(_M_depth_attachment_renference != nullptr);

        if (layout < layouts_count)
        {
            _M_clear_values[layout].setColor(
                    vk::ClearColorValue(Array<float, 4>({color.x, color.y, color.z, color.a})));
        }
        else
        {
            vulkan_debug_log("Vulkan API: Incorrect layout index!");
        }
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::clear_depth_stencil(const DepthStencilClearValue& value)
    {
        if (_M_depth_attachment_renference)
        {
            _M_clear_values.back().setDepthStencil(vk::ClearDepthStencilValue(value.depth, value.stencil));
        }
        return *this;
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
        DESTROY_CALL(destroyRenderPass, _M_render_pass);
    }
}// namespace Engine
