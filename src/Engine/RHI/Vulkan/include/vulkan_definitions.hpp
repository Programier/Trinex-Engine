#pragma once
#include <Core/definitions.hpp>
#include <Core/logger.hpp>


#define MAX_BINDLESS_RESOURCES 512
#define vulkan_error_log error_log
#define vulkan_debug_log debug_log
#define vulkan_info_log info_log
#define USE_INTEGRATED_GPU 0
#define API Engine::VulkanAPI::_M_vulkan
#define VIEW_PORT API->window_data.view_port

#define ENABLE_VALIDATION_LAYERS (VULKAN_DEBUG_BUILD && !PLATFORM_ANDROID)
#define MAX_BINDING_INDEX 15

/*
 *  TODO - I'm not sure that ignore the result of eSuboptimalKHR is a valid solution.
 *  This can slow down the API, so you need to be aware of it.
 *  This currently fixes the very low FPS issue on Android, so I'll leave it as is for now
 */

#define SKIP_SUBOPTIMAL_KHR_ERROR (PLATFORM_ANDROID)


#define DESTROY_CALL(func, instance)                                                                                             \
    {                                                                                                                            \
        if (instance)                                                                                                            \
            API->_M_device.func(instance);                                                                                       \
        instance = nullptr;                                                                                                      \
    }


#define VULKAN_FORCED_DESTROY_TYPES 255
#define VULKAN_VIEWPORT_ID (VULKAN_FORCED_DESTROY_TYPES + 1)
