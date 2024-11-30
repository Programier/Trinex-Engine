#pragma once
#include <Core/definitions.hpp>

#define API Engine::VulkanAPI::m_vulkan
#define VIEW_PORT API->window_data.view_port

#define ENABLE_VALIDATION_LAYERS (TRINEX_DEBUG_BUILD)
#define DESTROY_CALL(func, instance)                                                                                             \
	{                                                                                                                            \
		if (instance)                                                                                                            \
			API->m_device.func(instance);                                                                                        \
		instance = nullptr;                                                                                                      \
	}

#define VK_DESCRIPTOR_WAIT_FRAMES 300
#define VK_STAGGING_RESOURCE_WAIT_FRAMES 200
