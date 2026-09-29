#pragma once
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanContext;
}

namespace Trinex::Barrier
{

	void transition_image_layout(VulkanContext* ctx, vk::ImageMemoryBarrier& barrier);
}// namespace Trinex::Barrier
