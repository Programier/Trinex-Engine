#pragma once

#define DESTROY_CALL(func, obj)                                                                                                  \
	{                                                                                                                            \
		if (obj)                                                                                                                 \
			VulkanAPI::instance()->m_device.func(obj);                                                                           \
		obj = nullptr;                                                                                                           \
	}

#define VK_DESCRIPTOR_WAIT_FRAMES 300
#define VK_STAGGING_RESOURCE_WAIT_FRAMES 200
