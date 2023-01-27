#include <cstring>
#include <iostream>
#include <optional>
#include <set>
#include <vulkan_api.hpp>
#include <vulkan_export.hpp>

namespace Engine
{

    static const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    static const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#if ENABLE_VALIDATION_LAYERS

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                         VkDebugUtilsMessageTypeFlagsEXT message_ttype,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                         void* user_data)
    {
        std::cerr << "VulkanAPI: " << callback_data->pMessage << std::endl;
        return VK_FALSE;
    }

    static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
    {
        reset_vulkan_instance(create_info);
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
    }

#endif

    VulkanAPI* VulkanAPI::_M_vulkan = nullptr;

    API_EXPORT GraphicApiInterface::ApiInterface* load_api()
    {
        if (VulkanAPI::_M_vulkan == nullptr)
            VulkanAPI::_M_vulkan = new VulkanAPI();

        return VulkanAPI::_M_vulkan;
    }

    // INITIALIZE API

    static std::vector<const char*> get_required_extensions(SDL_Window* window)
    {
        std::vector<const char*> extensions;
        uint32_t count = 0;
        SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
        extensions.resize(count);
        SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
        return extensions;
    }

#if ENABLE_VALIDATION_LAYERS
    bool check_validation_layer_support()
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> availableLayers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, availableLayers.data());

        for (const char* layerName : validation_layers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (std::strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

#endif

    bool VulkanAPI::create_vulkan_instance()
    {
#if ENABLE_VALIDATION_LAYERS
        if (!check_validation_layer_support())
            return false;
#endif
        VkApplicationInfo app_info;
        reset_vulkan_instance(app_info);
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "GameEngine";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;
        app_info.pNext = nullptr;

        VkInstanceCreateInfo create_info{};
        reset_vulkan_instance(create_info);
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        auto extensions = get_required_extensions(_M_window);
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

#if ENABLE_VALIDATION_LAYERS
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        reset_vulkan_instance(debug_create_info);
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();

        populate_debug_messenger_create_info(debug_create_info);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
#else
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
#endif


        if (vkCreateInstance(&create_info, nullptr, &_M_vulkan_instance) != VK_SUCCESS)
        {
            return false;
        }

#if ENABLE_VALIDATION_LAYERS
        return set_debug_messenger();
#else
        return true;
#endif
    }


    QueueFamilyIndices VulkanAPI::find_queue_families(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

        int i = 0;
        for (const auto& queueFamily : queue_families)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphics_family = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _M_surface, &presentSupport);

            if (presentSupport)
            {
                indices.present_family = i;
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }


    bool VulkanAPI::check_device_extension_support(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(device_extensions.begin(), device_extensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails VulkanAPI::query_swap_chain_support(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _M_surface, &details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _M_surface, &format_count, nullptr);

        if (format_count != 0)
        {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _M_surface, &format_count, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _M_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _M_surface, &presentModeCount,
                                                      details.presentModes.data());
        }

        return details;
    }

    bool VulkanAPI::is_device_suitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = find_queue_families(device);

        bool extensionsSupported = check_device_extension_support(device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = query_swap_chain_support(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool VulkanAPI::create_physical_device()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(_M_vulkan_instance, &device_count, nullptr);

        if (device_count == 0)
        {
            //throw std::runtime_error("failed to find GPUs with Vulkan support!");
            return false;
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(_M_vulkan_instance, &device_count, devices.data());

        for (const auto& device : devices)
        {
            if (is_device_suitable(device))
            {
                _M_physical_device = device;
                break;
            }
        }

        if (_M_physical_device == VK_NULL_HANDLE)
        {
            //throw std::runtime_error("failed to find a suitable GPU!");
            return false;
        }
        return true;
    }

    bool VulkanAPI::create_surface()
    {
        SDL_Vulkan_CreateSurface(_M_window, _M_vulkan_instance, &_M_surface);
        return true;
    }

    bool VulkanAPI::create_device()
    {
        QueueFamilyIndices indices = find_queue_families(_M_physical_device);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};

        float queue_priority = 1.0f;
        for (uint32_t queueFamily : unique_queue_families)
        {
            VkDeviceQueueCreateInfo queueCreateInfo;
            reset_vulkan_instance(queueCreateInfo);
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures device_features;
        std::memset(&device_features, 0, sizeof(device_features));

        VkDeviceCreateInfo create_info;
        reset_vulkan_instance(create_info);
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.pEnabledFeatures = &device_features;
        create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        create_info.ppEnabledExtensionNames = device_extensions.data();
        create_info.enabledLayerCount = 0;
        create_info.flags = 0;
        create_info.pNext = nullptr;


        if (vkCreateDevice(_M_physical_device, &create_info, nullptr, &_M_device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(_M_device, indices.graphics_family.value(), 0, &_M_graphics_queue);
        vkGetDeviceQueue(_M_device, indices.present_family.value(), 0, &_M_present_queue);
        return true;
    }

    VulkanAPI::VulkanAPI()
    {
        SDL_Vulkan_LoadLibrary(nullptr);
    }

#if ENABLE_VALIDATION_LAYERS
    static VkResult create_debug_utils_messenger_ext(VkInstance instance,
                                                     const VkDebugUtilsMessengerCreateInfoEXT* create_info,
                                                     const VkAllocationCallbacks* allocator,
                                                     VkDebugUtilsMessengerEXT* messenger)
    {
        auto func =
                (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, create_info, allocator, messenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    bool VulkanAPI::set_debug_messenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT create_info;
        reset_vulkan_instance(create_info);
        populate_debug_messenger_create_info(create_info);

        if (create_debug_utils_messenger_ext(_M_vulkan_instance, &create_info, nullptr, &_M_debug_messenger) !=
            VK_SUCCESS)
        {
            //throw std::runtime_error("failed to set up debug messenger!");
            return true;
        }
        return true;
    }
#endif


    void* VulkanAPI::init_window(SDL_Window* window)
    {
        _M_window = window;
        auto funcs = {&VulkanAPI::create_vulkan_instance, &VulkanAPI::create_surface,
                      &VulkanAPI::create_physical_device, &VulkanAPI::create_device};

        for (auto func : funcs)
        {
            if (!(this->*func)())
            {
                return nullptr;
            }
        }

        return static_cast<void*>(window);
    }


#if ENABLE_VALIDATION_LAYERS
    static void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                  const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
#endif
    VulkanAPI::~VulkanAPI()
    {
        vkDestroyDevice(_M_device, nullptr);
#if ENABLE_VALIDATION_LAYERS
        destroy_debug_utils_messenger_ext(_M_vulkan_instance, _M_debug_messenger, nullptr);
#endif

        vkDestroySurfaceKHR(_M_vulkan_instance, _M_surface, nullptr);
        vkDestroyInstance(_M_vulkan_instance, nullptr);

        SDL_Vulkan_UnloadLibrary();
    }
}// namespace Engine
