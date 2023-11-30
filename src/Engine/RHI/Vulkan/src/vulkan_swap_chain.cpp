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
        vulkan_info_log("Vulkan API", "Creating new swapchain");
        vkb::SwapchainBuilder swapchain_builder(API->_M_bootstrap_device);
        if (API->_M_swap_chain)
        {
            swapchain_builder.set_old_swapchain(API->_M_swap_chain->_M_swap_chain);
        }

        swapchain_builder.set_desired_present_mode(static_cast<VkPresentModeKHR>(API->_M_swap_chain_mode));

        size_t images_count = glm::min(API->_M_surface_capabilities.minImageCount, API->_M_surface_capabilities.maxImageCount);
        swapchain_builder.set_desired_min_image_count(images_count).set_required_min_image_count(images_count);

        swapchain_builder.add_image_usage_flags(static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferSrc |
                                                                               vk::ImageUsageFlagBits::eTransferDst));
#if PLATFORM_ANDROID
        swapchain_builder.set_pre_transform_flags(VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR);
#endif
        VkSurfaceFormatKHR f;
        f.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        f.format     = VK_FORMAT_B8G8R8A8_UNORM;
        swapchain_builder.set_desired_format(f);

        auto swap_ret = swapchain_builder.build();

        if (!swap_ret)
        {
            throw std::runtime_error(swap_ret.error().message());
        }

        _M_bootstrap_swapchain = swap_ret.value();
        _M_swap_chain          = vk::SwapchainKHR(_M_bootstrap_swapchain.swapchain);

        _M_extent.width  = _M_bootstrap_swapchain.extent.width;
        _M_extent.height = _M_bootstrap_swapchain.extent.height;

        _M_format = vk::Format(_M_bootstrap_swapchain.image_format);

        _M_bootstrap_swapchain.get_images();
        auto images_ret = _M_bootstrap_swapchain.get_images();

        if (!images_ret)
        {
            throw std::runtime_error(images_ret.error().message());
        }

        _M_images.reserve(_M_bootstrap_swapchain.image_count);
        API->_M_framebuffers_count = _M_bootstrap_swapchain.image_count;

        for (auto& image : images_ret.value())
        {
            _M_images.push_back(vk::Image(image));
        }

        auto image_views_ret = _M_bootstrap_swapchain.get_image_views();

        if (!image_views_ret)
        {
            throw std::runtime_error(image_views_ret.error().message());
        }

        _M_image_views.reserve(_M_bootstrap_swapchain.image_count);

        for (auto& image_view : image_views_ret.value())
        {
            _M_image_views.push_back(vk::ImageView(image_view));
        }
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
