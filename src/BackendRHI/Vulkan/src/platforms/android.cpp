#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <android/native_window.h>

#include <vulkan_headers.hpp>

#include <Core/exception.hpp>
#include <vulkan/vulkan_android.h>

namespace Engine
{
	vk::SurfaceKHR create_vulkan_surface(void* native_window, vk::Instance instance)
	{
		ANativeWindow* window = reinterpret_cast<ANativeWindow*>(native_window);

		VkSurfaceKHR surface;
		VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType                         = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.window                        = window;

		if (vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS)
		{
			throw EngineException("Android: Failed to create Vulkan Surface");
		}

		return surface;
	}

	void load_required_extensions(void* native_window, Vector<String>& required_extensions)
	{
		required_extensions.push_back("VK_KHR_surface");
		required_extensions.push_back("VK_KHR_android_surface");
	}
}// namespace Engine
