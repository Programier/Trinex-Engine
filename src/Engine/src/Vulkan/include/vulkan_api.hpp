#pragma once
#include <Core/logger.hpp>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <api.hpp>
#include <vulkan/vulkan.h>
#include <vulkan_api.hpp>
#include <optional>
#include <cstring>

#define ENABLE_VALIDATION_LAYERS 0

#if ENABLE_VALIDATION_LAYERS
#define VALIDATION_LAYER_CODE(code) code
#else
#define VALIDATION_LAYER_CODE(code)
#endif

namespace Engine
{
    template<typename Type>
    void reset_vulkan_instance(Type& instance)
    {
        std::memset(&instance, 0, sizeof(instance));
    }

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        inline bool isComplete()
        {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct VulkanAPI : public GraphicApiInterface::ApiInterface {
        static VulkanAPI* _M_vulkan;
        Logger* _M_current_logger = nullptr;
        SDL_Window* _M_window = nullptr;

        VkInstance _M_vulkan_instance = nullptr;
        VkPhysicalDevice _M_physical_device = nullptr;
        VkSurfaceKHR _M_surface = nullptr;
#if ENABLE_VALIDATION_LAYERS
        VkDebugUtilsMessengerEXT _M_debug_messenger = nullptr;
#endif

        VkDevice _M_device = nullptr;
        VkQueue _M_graphics_queue = nullptr;
        VkQueue _M_present_queue = nullptr;


    private:
        bool create_vulkan_instance();
        bool create_physical_device();
        bool create_surface();
        bool create_device();
#if ENABLE_VALIDATION_LAYERS
        bool set_debug_messenger();
#endif

        QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
        bool is_device_suitable(VkPhysicalDevice device);
        bool check_device_extension_support(VkPhysicalDevice device);
        SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);

    public:
        VulkanAPI();


        ///////////////////////////////
        void* init_window(SDL_Window* window);
        ~VulkanAPI();
    };
}// namespace Engine
