#pragma once
#include <vulkan_headers.hpp>

namespace Engine::Barrier
{
	void transition_image_layout(vk::ImageMemoryBarrier& barrier);
	void transition_buffer_access(vk::BufferMemoryBarrier& barrier);
}// namespace Engine::Barrier
