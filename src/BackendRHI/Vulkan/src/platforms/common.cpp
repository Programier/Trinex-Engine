#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan_api.hpp>

namespace Trinex
{
	VkSurfaceKHR create_vulkan_surface(void* native_window, VkInstance instance)
	{
		SDL_Window* window = reinterpret_cast<SDL_Window*>(native_window);

		VkSurfaceKHR surface = VK_NULL_HANDLE;

		if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
		{
			return VK_NULL_HANDLE;
		}

		return surface;
	}

	void load_required_extensions(void* native_window, Vector<String>& required_extensions)
	{
		Uint32 count                  = 0;
		const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);

		if (!extensions)
		{
			return;
		}

		required_extensions.reserve(required_extensions.size() + count);

		for (Uint32 i = 0; i < count; ++i)
		{
			required_extensions.emplace_back(extensions[i]);
		}
	}
}// namespace Trinex
