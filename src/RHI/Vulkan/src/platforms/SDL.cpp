#if PLATFORM_LINUX
	#define VK_USE_PLATFORM_WAYLAND_KHR
	#define VK_USE_PLATFORM_XLIB_KHR
#elif PLATFORM_WINDOWS
	#include <Windows.h>
	#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <Core/window.hpp>
#include <SDL3/SDL_video.h>
#include <vulkan_api.hpp>
#include <wayland-client.h>

#if PLATFORM_LINUX

#endif

namespace Trinex
{
	template<typename T>
	static T load_function(vk::Instance instance, const char* name)
	{
		return reinterpret_cast<T>(instance.getProcAddr(name));
	}

#if PLATFORM_LINUX
	static vk::SurfaceKHR create_wayland_surface(SDL_Window* window, vk::Instance instance)
	{
		const SDL_PropertiesID properties = SDL_GetWindowProperties(window);

		if (properties == 0)
		{
			trinex_failure_fmt("SDL_GetWindowProperties failed: %s", SDL_GetError());
		}

		auto* display =
		        static_cast<wl_display*>(SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));

		auto* surface =
		        static_cast<wl_surface*>(SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));

		if (display == nullptr)
		{
			trinex_failure_msg("Failed to get Wayland wl_display from SDL window");
		}

		if (surface == nullptr)
		{
			trinex_failure_msg("Failed to get Wayland wl_surface from SDL window");
		}

		const auto create_surface = load_function<PFN_vkCreateWaylandSurfaceKHR>(instance, "vkCreateWaylandSurfaceKHR");
		trinex_verify_msg(create_surface, "VK_KHR_wayland_surface is not enabled");

		VkWaylandSurfaceCreateInfoKHR info = {VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};
		info.display                       = display;
		info.surface                       = surface;

		VkSurfaceKHR surface_handle = VK_NULL_HANDLE;
		const VkResult result       = create_surface(static_cast<VkInstance>(instance), &info, nullptr, &surface_handle);

		if (result != VK_SUCCESS)
		{
			trinex_failure_fmt("Failed to create Vulkan Wayland surface, error code: %d", static_cast<i32>(result));
		}

		return vk::SurfaceKHR(surface_handle);
	}

	static vk::SurfaceKHR create_x11_surface(SDL_Window* window, vk::Instance instance)
	{
		const SDL_PropertiesID properties = SDL_GetWindowProperties(window);

		if (properties == 0)
		{
			trinex_failure_fmt("SDL_GetWindowProperties failed: %s", SDL_GetError());
		}

		auto* display = static_cast<Display*>(SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));

		const ::Window x11_window =
		        static_cast<::Window>(SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));

		if (display == nullptr)
		{
			trinex_failure_msg("Failed to get X11 Display from SDL window");
		}

		if (x11_window == 0)
		{
			trinex_failure_msg("Failed to get X11 Window from SDL window");
		}

		const auto create_surface = load_function<PFN_vkCreateXlibSurfaceKHR>(instance, "vkCreateXlibSurfaceKHR");
		trinex_verify_msg(create_surface, "VK_KHR_xlib_surface is not enabled");

		VkXlibSurfaceCreateInfoKHR info = {VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};
		info.dpy                        = display;
		info.window                     = x11_window;

		VkSurfaceKHR surface_handle = VK_NULL_HANDLE;
		const VkResult result       = create_surface(static_cast<VkInstance>(instance), &info, nullptr, &surface_handle);

		if (result != VK_SUCCESS)
		{
			trinex_failure_fmt("Failed to create Vulkan X11 surface, error code: %d", static_cast<i32>(result));
		}

		return vk::SurfaceKHR(surface_handle);
	}

	static vk::SurfaceKHR create_window_surface(SDL_Window* window, vk::Instance instance)
	{
		const char* driver = SDL_GetCurrentVideoDriver();

		if (SDL_strcmp(driver, "wayland") == 0)
		{
			return create_wayland_surface(window, instance);
		}
		else if (SDL_strcmp(driver, "x11") == 0)
		{
			return create_x11_surface(window, instance);
		}

		return {};
	}
#elif PLATFORM_WINDOWS
	static vk::SurfaceKHR create_window_surface(SDL_Window* window, vk::Instance instance)
	{
		const SDL_PropertiesID properties = SDL_GetWindowProperties(window);

		if (properties == 0)
		{
			trinex_failure_fmt("SDL_GetWindowProperties failed: %s", SDL_GetError());
		}

		auto* hwnd = static_cast<HWND>(SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

		auto* hinstance =
		        static_cast<HINSTANCE>(SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr));

		if (hwnd == nullptr)
		{
			trinex_failure_msg("Failed to get Win32 HWND from SDL window");
		}

		if (hinstance == nullptr)
		{
			trinex_failure_msg("Failed to get Win32 HINSTANCE from SDL window");
		}

		const auto create_surface = load_function<PFN_vkCreateWin32SurfaceKHR>(instance, "vkCreateWin32SurfaceKHR");
		trinex_verify_msg(create_surface, "VK_KHR_win32_surface is not enabled");

		VkWin32SurfaceCreateInfoKHR info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
		info.hwnd                        = hwnd;
		info.hinstance                   = hinstance;

		VkSurfaceKHR surface_handle = VK_NULL_HANDLE;
		const VkResult result       = create_surface(static_cast<VkInstance>(instance), &info, nullptr, &surface_handle);

		if (result != VK_SUCCESS)
		{
			trinex_failure_fmt("Failed to create Vulkan Win32 surface, error code: %d", static_cast<i32>(result));
		}

		return vk::SurfaceKHR(surface_handle);
	}
#else
	#error "Implementation required"
#endif

	vk::SurfaceKHR VulkanAPI::create_surface(Window* window)
	{
		return create_window_surface(static_cast<SDL_Window*>(window->native_handle()), m_instance.instance);
	}
} // namespace Trinex
