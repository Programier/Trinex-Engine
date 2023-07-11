#pragma once

#define MAX_BINDLESS_RESOURCES 512
#define vulkan_debug_log(...) (*(API->_M_engine_logger))->error(__VA_ARGS__)
#define MAIN_FRAMEBUFFERS_COUNT 2
#define API VulkanAPI::_M_vulkan
#define VIEW_PORT API->window_data.view_port
#define DEFAULT_PRESENT_MODE vk::PresentModeKHR::eImmediate

#define ENABLE_VALIDATION_LAYERS 0
