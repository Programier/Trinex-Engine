#include <SDL.h>
#include <SDL_vulkan.h>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	VkSurfaceKHR create_vulkan_surface(void* native_window, VkInstance instance)
	{
		SDL_Window* window   = reinterpret_cast<SDL_Window*>(native_window);
		VkSurfaceKHR surface = {};
		SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), &surface);
		return surface;
	}

	void load_required_extensions(void* native_window, Vector<String>& required_extensions)
	{
		SDL_Window* window = reinterpret_cast<SDL_Window*>(native_window);
		unsigned int count = 0;
		SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);

		Vector<const char*> extentions(count, nullptr);
		SDL_Vulkan_GetInstanceExtensions(window, &count, extentions.data());

		for (const char* extention : extentions)
		{
			required_extensions.push_back(extention);
		}
	}
}// namespace Engine
