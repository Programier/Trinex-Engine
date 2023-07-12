#pragma once
#include <Core/predef.hpp>


#define MAX_BINDLESS_RESOURCES 512
#define vulkan_error_log(...) (*(API->_M_engine_logger))->error(__VA_ARGS__)
#define vulkan_debug_log(...) (*(API->_M_engine_logger))->debug(__VA_ARGS__)
#define vulkan_info_log(...) (*(API->_M_engine_logger))->log(__VA_ARGS__)
#define MAIN_FRAMEBUFFERS_COUNT 2
#define API VulkanAPI::_M_vulkan
#define VIEW_PORT API->window_data.view_port
#define DEFAULT_PRESENT_MODE vk::PresentModeKHR::eImmediate

#define ENABLE_VALIDATION_LAYERS (VULKAN_DEBUG_BUILD && !PLATFORM_ANDROID)

/*
 *  TODO - I'm not sure that ignore the result of eSuboptimalKHR is a valid solution.
 *  This can slow down the API, so you need to be aware of it.
 *  This currently fixes the very low FPS issue on Android, so I'll leave it as is for now
 */

#define SKIP_SUBOPTIMAL_KHR_ERROR (PLATFORM_ANDROID)
