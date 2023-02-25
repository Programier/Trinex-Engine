#include <vulkan_api.hpp>
#include <vulkan_framebuffer.hpp>


namespace Engine
{

    VulkanFramebuffer& VulkanFramebuffer::generate()
    {
        return *this;
    }

    void* VulkanFramebuffer::get_instance_data()
    {
        return static_cast<void*>(this);
    }

    VulkanFramebuffer& VulkanFramebuffer::init_render_pass_info()
    {
        _M_render_pass_info.setRenderPass(_M_render_pass);
        _M_render_pass_info.setFramebuffer(_M_framebuffer);
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::clear_color(BufferType type)
    {
        if (type == 0)
            return *this;

        _M_render_pass_info.setPClearValues(API->_M_clear_values);
        _M_render_pass_info.clearValueCount = 0;

        if (type & BufferBitType::ColorBufferBit)
        {
            ++_M_render_pass_info.clearValueCount;
        }

        if (type & BufferBitType::DepthBufferBit)
        {
            ++_M_render_pass_info.clearValueCount;
        }
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::update_viewport()
    {
        _M_render_pass_info.setRenderArea(
                vk::Rect2D({static_cast<std::int32_t>(VIEW_PORT.x), static_cast<std::int32_t>(VIEW_PORT.y)},
                           vk::Extent2D(API->_M_swap_chain->_M_extent.width, API->_M_swap_chain->_M_extent.height)));
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::destroy()
    {
        for (auto& image_view : _M_image_views)
        {
            API->_M_device.destroyImageView(image_view._M_image_view);
        }

        _M_image_views.clear();
        API->_M_device.destroyFramebuffer(_M_framebuffer);
        return *this;
    }

    VulkanFramebuffer& VulkanFramebuffer::bind()
    {
        if (API->_M_current_framebuffer)
        {
            API->_M_current_framebuffer->unbind();
        }

        API->_M_current_framebuffer = this;
        return update_viewport();
    }

    VulkanFramebuffer& VulkanFramebuffer::unbind()
    {
        API->_M_current_framebuffer = nullptr;
        return *this;
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        destroy();

        if (_M_destroy_render_pass)
        {
            API->_M_device.destroyRenderPass(_M_render_pass);
        }
    }
}// namespace Engine
