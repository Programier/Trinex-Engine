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
	extern bool require_present;
	extern bool require_dedicated_transfer_queue;
	extern bool require_dedicated_compute_queue;
	extern bool require_separate_transfer_queue;
	extern bool require_separate_compute_queue;
	extern unsigned int required_mem_size;
	extern unsigned int desired_mem_size;
}// namespace Engine::VulkanConfig
