#include <Core/definitions.hpp>
#include <VkBootstrap.h>
#include <algorithm>
#include <vulkan_api.hpp>
#include <vulkan_swap_chain.hpp>

#define INIT_FRAMEBUFFERS_COUNT 3

namespace Engine
{
    SwapChain::SwapChain()
    {

    }
    SwapChain::~SwapChain()
    {
        vulkan_info_log("Vulkan API", "Destroy swapchain");
        for (auto& view : _M_image_views)
        {
            API->_M_device.destroyImageView(view);
        }
        vkb::destroy_swapchain(_M_bootstrap_swapchain);
    }

}// namespace Engine
