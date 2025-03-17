#pragma once
#include <vulkan_headers.hpp>

namespace Engine::Barrier
{
	void transition_image_layout(vk::ImageMemoryBarrier& barrier);
}// namespace Engine::Barrier
