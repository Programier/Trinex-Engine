#pragma once

#include <vulkan_headers.hpp>
#include <VkBootstrap.h>


namespace Engine
{
    struct SwapChain {
        vk::SwapchainKHR _M_swap_chain;
        vkb::Swapchain _M_bootstrap_swapchain;

        vk::Extent2D _M_extent;
        Vector<vk::Image> _M_images;
        Vector<vk::ImageView> _M_image_views;
        vk::Format _M_format;

        SwapChain();
        ~SwapChain();        
    };

}// namespace Engine
