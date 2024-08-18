#pragma once
#include <Core/etl/stl_wrapper.hpp>
#include <VkBootstrap.h>
#include <vulkan_headers.hpp>


namespace Engine
{
	struct SwapChain {
		vk::SwapchainKHR m_swap_chain;
		vkb::Swapchain m_bootstrap_swapchain;

		vk::Extent2D m_extent;
		Vector<vk::Image> m_images;
		Vector<vk::ImageView> m_image_views;
		vk::Format m_format;

		SwapChain();
		~SwapChain();
	};

}// namespace Engine
