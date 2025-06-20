#pragma once

namespace vkb
{
	enum class PreferredDeviceType;
}

namespace Engine::VulkanConfig
{
	extern vkb::PreferredDeviceType device_type;
	extern bool enable_validation;
	extern bool allow_any_gpu_type;
}// namespace Engine::VulkanConfig
