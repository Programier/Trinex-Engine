#pragma once
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanContext;
}

namespace Engine::Barrier
{

	void transition_image_layout(VulkanContext* ctx, vk::ImageMemoryBarrier& barrier);
}// namespace Engine::Barrier
