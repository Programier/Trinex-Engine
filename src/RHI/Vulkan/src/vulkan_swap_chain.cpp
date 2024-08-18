#include <Core/definitions.hpp>
#include <VkBootstrap.h>
#include <algorithm>
#include <vulkan_api.hpp>
#include <vulkan_swap_chain.hpp>

#define INIT_FRAMEBUFFERS_COUNT 3

namespace Engine
{
	SwapChain::SwapChain()
	{}
	SwapChain::~SwapChain()
	{
		vulkan_info_log("Vulkan API", "Destroy swapchain");
		for (auto& view : m_image_views)
		{
			API->m_device.destroyImageView(view);
		}
		vkb::destroy_swapchain(m_bootstrap_swapchain);
	}

}// namespace Engine
