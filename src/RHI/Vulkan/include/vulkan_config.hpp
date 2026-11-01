#pragma once
#include <vulkan_headers.hpp>

namespace Trinex::VulkanConfig
{
	enum class GPUType : i32
	{
		Other      = VK_PHYSICAL_DEVICE_TYPE_OTHER,
		Integrated = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
		Discrete   = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
		Virtual    = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
		CPU        = VK_PHYSICAL_DEVICE_TYPE_CPU,
	};

	extern GPUType device_type;
	extern bool enable_validation;
	extern bool allow_any_gpu_type;
}// namespace Trinex::VulkanConfig
