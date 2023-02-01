#pragma once

#include <vulkan/vulkan.hpp>
#include <VkBootstrap.h>


namespace Engine
{
    struct SwapChain {
        vk::SwapchainKHR _M_swap_chain;
        vkb::Swapchain _M_bootstrap_swapchain;

        VkExtent2D _M_extent;
        std::vector<vk::Image> _M_images;
        std::vector<vk::ImageView> _M_image_views;
        vk::Format _M_format;

        SwapChain();
        ~SwapChain();
    };

}// namespace Engine
